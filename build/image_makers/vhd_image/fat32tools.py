import struct
import codecs
import os

def log(*args):
    return
    print(*args)

class BootSector(object):
    def __init__(self, total_partition_sectors = None, reserved_sectors = 0x4000, boot_code = b"\x00"*(0x200-2-90),
                 oem_name = b"QubFat32", 
                 volume_id = 0x12345678,
                 volume_label = b"Qube-volume",
                 hidden_sectors_count = 0
                 ):
        if total_partition_sectors == None:
            return
        assert reserved_sectors < total_partition_sectors
        self.jmp_inst = b"\xeb\x58\x90"
        self.oem_name = oem_name
        self.mirroring_flags = 0
        self.version = 0
        self.root_dir_cluster = 2
        self.fs_info_sector = 1
        self.boot_sectors_copy_sector_number = 6
        self.reserved = b"\x00"*12
        self.phys_drive_number = 0x80 # fixed drive
        self.reserved2 = 0
        self.extended_boot_sig = 0x29
        self.volume_id = volume_id
        self.volume_label = volume_label
        self.fs_type = b"FAT32   "
        self.sectors_per_cluster = 2
        self.reserved_sectors = reserved_sectors
        spc = self.sectors_per_cluster
        rs = reserved_sectors
        sectors_left = total_partition_sectors - rs
        log("reserved sectors:", hex(reserved_sectors))
        log("sectors left: ", hex(sectors_left))
        self.sectors_per_fat = -(-sectors_left//(1+64*spc)//2)
        log("self.sectors_per_fat",hex(self.sectors_per_fat))
        spf = self.sectors_per_fat
        self.phys_sectors_per_track = 63
        self.heads_count = 255
        #self.total_logical_sectors = spf*128*spc + spf*2 + rs
        self.total_logical_sectors = total_partition_sectors
        self.hidden_sectors_count = hidden_sectors_count
        self.bytes_per_sector = 0x200
        self.fat_count = 2
        self.depricated1 = 0
        self.depricated2 = 0
        self.fat_id = 0xf8 # fixed disk
        self.depricated3 = 0
        self.boot_code = boot_code.ljust(0x200-2-90,b"\x00")
        assert len(self.boot_code) == 0x200-2-90
    
    def from_data(self, data):
        assert len(data) == 0x200
        self.jmp_inst = data[:3]
        self.oem_name = data[3:11]
        [self.bytes_per_sector, self.sectors_per_cluster,
         self.reserved_sectors, self.fat_count, 
         self.depricated1, self.depricated2,
         self.fat_id, self.depricated3] = struct.unpack("<HBHBHHBH", data[11:24])
        [self.phys_sectors_per_track, self.heads_count, 
         self.hidden_sectors_count, self.total_logical_sectors] = struct.unpack("<HHII", data[24:36])
        [self.sectors_per_fat, self.mirroring_flags,
         self.version, self.root_dir_cluster, 
         self.fs_info_sector, 
         self.boot_sectors_copy_sector_number,
         self.reserved, self.phys_drive_number, self.reserved2,
         self.extended_boot_sig, self.volume_id, self.volume_label,
         self.fs_type] = struct.unpack(b"<IHHIHH12sBBBI11s8s", data[36:90])
        self.boot_code = data[90:0x200-2]
        assert data[0x200-2:] == b"\x55\xaa",(repr(data[0x200-2:]))
        self.partition_size = 512 * (self.total_logical_sectors+
                                     self.hidden_sectors_count)
        
        return self
        
    def dump(self):
        ret = self.jmp_inst + \
              self.oem_name + \
              struct.pack("<HBHBHHBH", self.bytes_per_sector, self.sectors_per_cluster,
                                       self.reserved_sectors, self.fat_count, 
                                       self.depricated1, self.depricated2,
                                       self.fat_id, self.depricated3) + \
              struct.pack("<HHII", self.phys_sectors_per_track, self.heads_count, 
                                   self.hidden_sectors_count, self.total_logical_sectors) + \
              struct.pack(b"<IHHIHH12sBBBI11s8s", self.sectors_per_fat, self.mirroring_flags,
                                                  self.version, self.root_dir_cluster, 
                                                  self.fs_info_sector, 
                                                  self.boot_sectors_copy_sector_number,
                                                  self.reserved, self.phys_drive_number, self.reserved2,
                                                  self.extended_boot_sig, self.volume_id, self.volume_label,
                                                  self.fs_type) + \
              self.boot_code + b"\x55\xaa"
        assert len(ret) == 0x200
        return ret
    
class FSInfo(object):
    def __init__(self):
        self.data = b"RRaA" + b"\x00"*(0x200-32) + b"rrAa" + b"\xff\xff\xff\xff\x02" + b"\x00"*17 + b"\x55\xaa"
    def from_data(self,data):
        self.data = data
        return self
    def dump(self):
        return self.data
class FileAllocationTable(object):
    def __init__(self, data):
        fat_size = len(data)
        assert fat_size % 4 == 0
        self.clusters_count = fat_size // 4
        self.entries = list(struct.unpack("<"+"I"*self.clusters_count, data))
        self.entries[0] = 0xffffff8
        self.entries[1] = 0xffffffff
        
        #log(self.entries[:10])
    def dump(self):
        return struct.pack("<" + "I"*self.clusters_count, *self.entries)
    
    def get_cluster_chain(self, cluster_id):
        log("get cluster_chain: %d"%cluster_id)
        if cluster_id == 0:
            return []
        next = self.entries[cluster_id]
        if next & 0xfffffff > 0xffffff8:
            return [cluster_id]
        return [cluster_id] + self.get_cluster_chain(next)
        
    def create_clusters_chain(self, clusters_count):
        log("create_clusters_chain: %d"%clusters_count)
        try:
            id = self.entries.index(0)
        except:
            log("No space left")
            raise Exception("No space left!")
        new_cluster = id
        log("add new cluster: %d"%new_cluster)
        if clusters_count == 1:
            log("last cluaster")
            self.entries[new_cluster] = 0xfffffff # last cluster
        else:
            self.entries[new_cluster] = 0xffffffff # save it
            self.entries[new_cluster] = self.create_clusters_chain(clusters_count-1)
        return new_cluster
    
    def free_clusters_chain(self, cluster_id):
        next = self.entries[cluster_id]
        if next & 0xfffffff > 0xffffff8:
            return
        self.entries[cluster_id] = 0xfffffff
        return self.free_cluster_chain(next)
        
class DirEntry(object):
    ATTR_RO = 1
    ATTR_HIDDEN = 2
    ATTR_SYSTEM = 4
    ATTR_VOL_LABEL = 8
    ATTR_SUBDIR = 0x10
    ATTR_ARCHIVE = 0x20
    ATTR_DEVICE = 0x40
    ATTR_RESERVED = 0x80
    def __init__(self, name = None, is_subdir = None, first_cluster = None, file_size = None):
        if name == None:
            return
        if b"." in name:
            name,ext = name.split(b".")
        else:
            ext = b""
        #import pdb
        #pdb.set_trace()
        
        name = name.ljust(8,b' ')
        ext = ext.ljust(3, b' ')
        name = name + ext
        assert len(name) == 11, (name, len(name))
        self.name = name
        assert isinstance(self.name, bytes) 
        if is_subdir:
            self.attr = DirEntry.ATTR_SUBDIR
        else:
            self.attr = 0
        self.reserved = 0
        self.create_time_tenth = 0
        self.create_time = 0
        self.create_date = 0
        self.last_access_date = 0
        self.first_cluster_high = first_cluster >> 16
        self.write_time = 0
        self.write_date = 0 
        self.first_cluster_low = first_cluster & 0xffff
        self.file_size = file_size
    def get_size(self):
        return self.file_size
    def from_data(self, data):
        [self.name,
         self.attr,
         self.reserved,
         self.create_time_tenth,
         self.create_time,
         self.create_date,
         self.last_access_date,
         self.first_cluster_high,
         self.write_time,
         self.write_date,
         self.first_cluster_low,
         self.file_size] = struct.unpack(b"<11sBBBHHHHHHHI", data)
        #self.name = self.name.decode("ascii")
        return self
    def dump(self):
        return struct.pack(b"<11sBBBHHHHHHHI", self.name,
                                        self.attr,
                                        self.reserved,
                                        self.create_time_tenth,
                                        self.create_time,
                                        self.create_date,
                                        self.last_access_date,
                                        self.first_cluster_high,
                                        self.write_time,
                                        self.write_date,
                                        self.first_cluster_low,
                                        self.file_size)
    def __str__(self):
        if self.name[0] == 0xe5:
            return "<DELETED_FILE>"
        return self.get_name().decode("ascii") + "(c:%d, s:%d)"%(self.get_cluster_id(),self.get_size())
    def plog(self):
        log("self.name,             ",self.name,
                                 "\n\tself.attr,             ",self.attr,
                                 "\n\tself.reserved,         ",self.reserved,
                                 "\n\tself.create_time_tenth,",self.create_time_tenth,
                                 "\n\tself.create_time,      ",self.create_time,
                                 "\n\tself.create_date,      ",self.create_date,
                                 "\n\tself.last_access_date, ",self.last_access_date,
                                 "\n\tself.first_cluster_high",self.first_cluster_high,
                                 "\n\tself.write_time,       ",self.write_time,
                                 "\n\tself.write_date,       ",self.write_date,
                                 "\n\tself.first_cluster_low,",self.first_cluster_low,
                                 "\n\tself.file_size]        ",self.file_size)
    def get_name(self):
        if self.name[0] == b'.'[0]:
            if self.name[1] == b'.'[0]:
                return b'..'
            return b'.'
        name = self.name[:8].strip()
        ext = self.name[8:].strip()
        if len(ext) == 0:
            return name
        return b".".join([name,ext])
    
    def is_folder(self):
        return 0 != self.attr & DirEntry.ATTR_SUBDIR
    def is_lfn(self):
        return self.attr == 0xf
    def get_cluster_id(self):
        return (self.first_cluster_high << 16) + self.first_cluster_low
    
    def set_cluster_id(self, first_cluster):
        self.first_cluster_high = first_cluster >> 16
        self.first_cluster_low = first_cluster & 0xffff
        
class FD(object):
    def __init__(self, dir_cluster_id, entry, dir_entry_offset):
        self.entry = entry
        self.dir_cluster_id = dir_cluster_id
        self.dir_entry_offset = dir_entry_offset
class FakeFd(object):
    def __init__(self, cluster_id):
        class FakeEntry(object):
            def __init__(self, cluster_id):
                self.cluster_id = cluster_id
                self.file_size = "asdf"
                self.init = True
                
            def is_folder(self):
                return True
                
            def get_cluster_id(self):
                return self.cluster_id
            def __setattr__(self, key, val):
                if 'init' in self.__dict__.keys():
                    raise Exception("Try to write to fake entry: %s=%s"%(str(key),str(val)))
                self.__dict__[key] = val
        self.entry = FakeEntry(cluster_id)
        self.init = True
        def __setattr__(self, key, val):
            if 'init' in self.__dict__.keys():
                raise Exception("Try to write to fake fd: %s=%s"%(str(key),str(val)))
            self.__dict__[key] = val
class Fat32(object):
    def __init__(self):
        pass
    
    def parse(self, path, offset = 0, end = 0xffffffff):
        log("Parse fat32. offset=0x%x"%offset)
        self.offset = offset
        self.f = open(path, "rb")
        self.f.seek(0,2)
        file_size = self.f.tell()
        log("file_size = %d"%file_size)
        self.f.seek(0,0)
        self.f.seek(self.offset)
        self.boot_sector = BootSector().from_data(self.f.read(0x200))
        assert self.boot_sector.boot_sectors_copy_sector_number == 6
        self.fs_info = FSInfo().from_data(self.f.read(0x200))
        reserved_count = self.boot_sector.reserved_sectors - 8
        self.sector2 = self.f.read(0x200)
        self.sector3 = self.f.read(0x200)
        self.sector4 = self.f.read(0x200)
        self.sector5 = self.f.read(0x200)
        self.boot_sector2 = BootSector().from_data(self.f.read(0x200))
        self.fs_info = FSInfo().from_data(self.f.read(0x200))
        assert reserved_count >= 0
        self.reserved_sectors = self.f.read(0x200 * reserved_count)
        assert len(self.reserved_sectors) == 0x200 * reserved_count
        assert self.boot_sector.fat_count == 2
        
        tls = self.boot_sector.total_logical_sectors
        spc = self.boot_sector.sectors_per_cluster
        hsc = self.boot_sector.hidden_sectors_count
        self.partition_sectors_count = (tls)
        #assert self.partition_sectors_count*512 <= file_size - self.offset, (self.partition_sectors_count*512,file_size,self.offset)
        self.assert_lengths()
        sectors_per_fat = self.boot_sector.sectors_per_fat
        #log(self.f.tell())
        self.fat1 = FileAllocationTable(self.f.read(0x200*sectors_per_fat))
        self.fat2 = FileAllocationTable(self.f.read(0x200*sectors_per_fat))
        self.data_clusters = []
        while True:
            if self.f.tell() >= end:
                break
            cluster = self.f.read(0x200*spc)
            if len(cluster) != 0x200*spc:
                break
            
            self.data_clusters.append(cluster)
        return self
    def dump(self):
        return self.boot_sector.dump() + \
                self.fs_info.dump() + \
                self.sector2+self.sector3+self.sector4+self.sector5+self.boot_sector.dump()+self.fs_info.dump() +\
                self.reserved_sectors + \
                self.fat1.dump() + \
                self.fat1.dump() + \
                b"".join(self.data_clusters)
    def sync(self, path):
        f = open(path, "wb")
        f.write(self.dump())
        f.close()
        tmp = self.data_clusters
        self.parse(path)
        #import code
        #code.interact(local = locals())
        
    def assert_lengths(self):
        tls = self.boot_sector.total_logical_sectors
        spc = self.boot_sector.sectors_per_cluster
        rs =  self.boot_sector.reserved_sectors
        spf = self.boot_sector.sectors_per_fat
        hsc = self.boot_sector.hidden_sectors_count
        fc =  self.boot_sector.fat_count
        #import code
        #code.interact(local = locals())
        #assert tls == self.partition_sectors_count, (tls, self.partition_sectors_count)
        clusters_count = spf * 512 / 4
        #assert tls == rs + spf * fc + clusters_count * spc
        
    def format(self, partition_sectors_count, oem_name = b"QubFat32", reserved_sectors_count = 0x4000,
                     volume_id = 0x12345678,
                     volume_label = b"Qube-volume",
                     hidden_sectors_count = 0x0, boot_code = b"\x00"*(0x200-2-90)):
        self.partition_sectors_count = partition_sectors_count
        self.boot_sector = BootSector(partition_sectors_count, oem_name = oem_name, reserved_sectors = reserved_sectors_count,
                                      volume_id = volume_id, volume_label = volume_label, hidden_sectors_count = hidden_sectors_count, boot_code = boot_code)
        self.fs_info = FSInfo()
        self.sector2 = b"\x00"*(0x200-2) + b"\x55\xaa"
        self.sector3 = b"\x00"*0x200
        self.sector4 = b"\x00"*0x200
        self.sector5 = b"\x00"*0x200
        
        reserved_count = self.boot_sector.reserved_sectors - 8
        log("reseverd count: %d"%reserved_count)
        assert reserved_count >= 0
        self.reserved_sectors = b"\x00" * 0x200 * reserved_count
        assert self.boot_sector.fat_count == 2
        self.assert_lengths()
        sectors_per_fat = self.boot_sector.sectors_per_fat
        self.fat1 = FileAllocationTable(b"\x00"*(0x200*sectors_per_fat))
        self.fat2 = FileAllocationTable(b"\x00"*(0x200*sectors_per_fat))
        spc = self.boot_sector.sectors_per_cluster
        self.data_clusters = [b"\x00"*(0x200*spc) for i in range(min(len(self.fat1.entries),(partition_sectors_count-reserved_sectors_count-sectors_per_fat*2)//spc))]
        log("kjg", len(self.fat1.entries),partition_sectors_count-reserved_sectors_count-sectors_per_fat*2)
        # Create the root:
        self.fat1.entries[self.boot_sector.root_dir_cluster] = 0xfffffff
        
        
        return self

    def mkdir(self, parent_cluster_id, new_dir_name):
        cur_dir_fd = FakeFd(parent_cluster_id)
        dir = Dir().from_data(self.read(cur_dir_fd, 0, 0xffffffff))
        new_entry = DirEntry(name = new_dir_name, is_subdir = True, first_cluster = 0, file_size = 0)
        dir_entry_offset = dir.add_file(new_entry)
        self.write(cur_dir_fd, 0, dir.dump())
        # write . and ..:
        fd = FD(parent_cluster_id, new_entry, dir_entry_offset)
        # create the first cluster:
        self.write(fd,0,b"\x00")
        
        self.write(fd,0,DirEntry(name = b".", is_subdir = True, first_cluster = fd.entry.get_cluster_id(), file_size = 0).dump())
        self.write(fd,32,DirEntry(name = b"..", is_subdir = True, first_cluster = fd.dir_cluster_id, file_size = 0).dump())
        return fd
        
    def create_file(self, dir_cluster_id, new_file_name):
        log("create_file called: parent_id=%d. file_name = %s"%(dir_cluster_id, new_file_name))
        fake_fd = FakeFd(dir_cluster_id)
        dir = Dir().from_data(self.read(fake_fd, 0, 0xffffffff))
        new_entry = DirEntry(name = new_file_name, is_subdir = False, first_cluster = 0, file_size = 0)
        dir.add_file(new_entry)
        self.write(fake_fd, 0, dir.dump())
        
    def open(self, path):
        orig_path = path
        assert path[0] == '/'
        path = path.encode("ascii")
        parts = [x for x in path.split(b"/") if x != b'']
        log("open called with: %s"%str(parts))
        cluster_id = self.boot_sector.root_dir_cluster
        # create dummy entry for the root, and dummy parent_cluster_id:
        entry = DirEntry(name = b"", is_subdir = True, first_cluster = 2, file_size = 0)
        entry_offset = 0
        #log("asdf: ", entry.is_folder())
        parent_cluster_id = None
        for i, path_part in enumerate(parts):
            cur_dir_fd = FD(parent_cluster_id, entry, entry_offset)
            if not entry.is_folder():
                raise Exception("Path part is not a directory: %s"%entry.get_name())
            entry_offset = 0
            for entry in Dir().from_data(self.read(cur_dir_fd, 0, 0xffffffff)).dir_entries:
                if entry.get_name() == path_part:
                    parent_cluster_id = cluster_id
                    cluster_id = entry.get_cluster_id()
                    break
                entry_offset += 32
            else: # not found.
                if i == len(parts)-1: # create new file
                    self.create_file(cluster_id, parts[-1])
                    log("returned from create file")
                    return self.open(orig_path)
                raise Exception("Path not found! (%s)"%path_part)
        
        fd = FD(dir_cluster_id = parent_cluster_id, entry = entry, dir_entry_offset = entry_offset)
        return fd
        
    def read(self, fd, offset, size):
        spc = self.boot_sector.sectors_per_cluster
        bpc = 0x200 * spc
        file_size = self.get_fd_size(fd)
        # Get the first cluster to read from:
        cluster_id = fd.entry.get_cluster_id()
        if cluster_id == 0:
            return b""
        while offset >= bpc:
            offset -= bpc
            file_size -= bpc
            cluster_id = self.get_next_cluster(cluster_id)
            if cluster_id == None: # end of file
                return b""
        ret = []
        
        while size >= bpc:
            size -= bpc
            ret.append(self.data_clusters[cluster_id-2])
            cluster_id = self.get_next_cluster(cluster_id)
            if cluster_id == None:
                break
        if cluster_id and size > 0:
            ret.append(self.data_clusters[cluster_id-2][:offset + size])
        ret = (b"".join(ret))
        if len(ret) > file_size:
            ret = ret[:file_size]
        ret = ret[offset:]
        return ret
    
    # Get file size. If diretory, returns the cluster_count * cluster_size
    def get_fd_size(self, fd):
        spc = self.boot_sector.sectors_per_cluster
        bpc = 0x200 * spc
        if fd.entry.is_folder():
            count = len(self.fat1.get_cluster_chain(fd.entry.get_cluster_id()))
            return bpc * count
        else:
            return fd.entry.file_size
        
    def write(self, fd, offset, data):
        spc = self.boot_sector.sectors_per_cluster
        bpc = 0x200 * spc
        orig_size = self.get_fd_size(fd)
        if orig_size < offset + len(data):
            self.resize(fd, offset + len(data))
        if orig_size < offset: # zero until offset:
            data = b"\x00"*(offset - orig_size) + data
            offset = orig_size
        
        # write the data:
        # Get the first cluster to write to:
        cluster_id = fd.entry.get_cluster_id()
        while offset >= bpc:
            offset -= bpc
            cluster_id = self.get_next_cluster(cluster_id)
            assert cluster_id != None # end of file
        size = len(data)
        i = 0
        while size > 0:
            # handle the edge cases:
            if offset > 0: # partial first cluster:
                first_cluster_size = bpc - offset
                assert first_cluster_size > 0
                cluster_data = self.data_clusters[cluster_id-2][:offset] + data[:first_cluster_size] + \
                               self.data_clusters[cluster_id-2][len(data[:first_cluster_size]) + offset:]
                assert len(cluster_data) == bpc, (len(self.data_clusters[cluster_id-2]), len(cluster_data),bpc)
                self.data_clusters[cluster_id-2] = cluster_data
                data = data[first_cluster_size:] # may be b''
                size -= first_cluster_size
            else: # regular
                new_data = data[i*bpc:(i+1)*bpc]
                self.data_clusters[cluster_id-2] = new_data + self.data_clusters[cluster_id-2][len(new_data):]
                i+=1
                size -= bpc
            if size > 0:
                cluster_id = self.get_next_cluster(cluster_id)
                assert cluster_id != None # end of file
            assert len(self.data_clusters[cluster_id-2]) == bpc
    def resize(self, fd, new_size):
        entry = fd.entry
        cur_size = self.get_fd_size(fd)
        spc = self.boot_sector.sectors_per_cluster
        bpc = 0x200 * spc
        if fd.entry.is_folder():
            assert new_size > 0
        cur_cluster_count = -(-cur_size // bpc)
        new_cluster_count = -(-new_size // bpc)
        if cur_cluster_count == new_cluster_count:
            return
        
        old_cluster_id = fd.entry.get_cluster_id()
        if new_cluster_count == 0:
            assert new_size == 0
            self.fat1.free_clusters_chain(old_cluster_id)
            entry.set_cluster_id(0)
            
        elif cur_cluster_count == 0:
            assert cur_size == 0
            assert entry.get_cluster_id() == 0
            assert new_cluster_count > 0
            new_cluster_id = self.fat1.create_clusters_chain(new_cluster_count)
            entry.set_cluster_id(new_cluster_id)
            
        elif cur_cluster_count < new_cluster_count:
            new_cluster_id = self.fat1.create_clusters_chain(new_cluster_count - cur_cluster_count)
            i = 1
            while True:
                tmp = self.get_next_cluster(old_cluster_id)
                if tmp == None:
                    break
                i+=1
            assert i == cur_cluster_count, (i,cur_cluster_count)
            assert self.fat1.entries[old_cluster_id]&0xfffffff >= 0xffffff8
            self.fat1.entries[old_cluster_id] = new_cluster_id
        else: # truncate:
            assert cur_cluster_count > 0
            last_cluster_id = old_cluster_id
            for i in range(cur_cluster_count-1):
                last_cluster_id = self.get_next_cluster(last_cluster_id)
            cluster_to_free = self.get_next_cluster(last_cluster_id)
            assert cluster_to_free != None
            self.fat1.free_clusters_chain(cluster_to_free)
            self.fat1.entries[last_cluster_id] = 0xfffffff
        if not entry.is_folder():
            entry.file_size = new_size
            dir_fd = FakeFd(fd.dir_cluster_id)
            self.write(dir_fd, fd.dir_entry_offset, entry.dump())
        
    def get_next_cluster(self, cluster_id):
        #log("get_next_cluster called: %x"%cluster_id)
        cluster_id = self.fat1.entries[cluster_id]
        if cluster_id&0xFFFFFFF < 0xFFFFFF8:
            return cluster_id
        return None
    
    def ls(self, path):
        log("ls %s"%path)
        fd = self.open(path)
        
        log(Dir().from_data(self.read(fd, 0, 0xffffffff)))
    
    """
    def read_data(self, cluster_id, file_size):
        #log("read_data from cluster: 0x%x"%cluster_id)
        clusters_ids = []
        while cluster_id&0xFFFFFFF < 0xFFFFFF8:
            clusters_ids.append(cluster_id)
            #log("read data from cluster: 0x%x"%cluster_id)
            #log(len(self.fat1.entries))
            cluster_id = self.fat1.entries[cluster_id]
        
        ret = [self.data_clusters[id-2] for id in clusters_ids]    
        return b"".join(ret)[:file_size]
    
    def write_data(self, cluster_id, data):
        bytes_per_cluster = 0x200 * self.boot_sector.sectors_per_cluster
        i = 0
        cur_cluster_id = cluster_id
        cluster_count = -(-len(data) // bytes_per_cluster)
        for i in range(cluster_count):
            cur_cluster_data = data[i*bytes_per_cluster:(i+1)*bytes_per_cluster]
            self.data_clusters[cur_cluster_id-2] = cur_cluster_data.ljust(bytes_per_cluster)
            cur_cluster_id = self.fat1.entries[cur_cluster_id]
            assert cur_cluster_id & 0xfffffff < 0xffffff8
        
   
    def read_dir(self, cluster_id, file_size):
        data = self.read_data(cluster_id, file_size)
        ret = []
        for entry in Dir().from_data(data).dir_entries:
            #log("read_dir:",entry.get_name())
            if entry.get_name()[0] == 0:
                break
            if entry.get_name()[0] == 0xe5:
                continue
            if entry.is_lfn():
                continue
            ret.append(entry)
        
        return ret
        
    def read_file(self, cluster_id, file_size):
        data = self.read_data(cluster_id, file_size)
        return data
    
    def read_dir_entry(self, dir_entry):
        log("Read entry %s (c:%d, s:%d)"%dir_entry.get_name(), dir_entry.get_cluster_id(), dir_entry.get_size())
        if dir_entry.is_folder():
            return self.read_dir(dir_entry.get_cluster_id(), dir_entry.get_size())
        else:
            return self.read_file(self, dir_entry.get_cluster_id(), dir_entry.get_size())
        
    def dump_folder(self, cluster_id = None, recurse = True, tabs = 0):
        if cluster_id == None:
            cluster_id = fat32.boot_sector.root_dir_cluster
        entries = self.read_dir(cluster_id, 0xffffffff)
        for entry in entries:
            name = entry.get_name()
            cluster_id = entry.get_cluster_id()
            if not entry.is_folder():
                #log("Read file %s (c:%d, s:%d)"%(entry.get_name(), entry.get_cluster_id(), entry.get_size()))
        
                data = self.read_file(entry.get_cluster_id(), entry.get_size())[:min(20, entry.get_size())]
                if entry.get_size() > 20:
                    data += b"[...]"
            else:
                data = "<DIR>"
            ttabs = " " * tabs
            size = entry.get_size()
            log(f"{ttabs}{name} (c:{cluster_id}, s:{size}): {data}")
            if entry.is_folder() and recurse and entry.get_name()[0] != 0x2e:
                self.dump_folder(entry.get_cluster_id(), recurse = True, tabs = tabs+1)
    """

class Dir(object):
    def __init__(self):
        self.dir_entries = []
    
    def from_data(self, file_data):
        assert len(file_data)%32 == 0, len(file_data)
        self.dir_entries = [DirEntry().from_data(file_data[x:x+32]) for x in range(0,len(file_data),32)]
        return self
        
    def add_file(self, dir_entry):
        assert isinstance(dir_entry, DirEntry)
        for i in range(len(self.dir_entries)):
            entry = self.dir_entries[i]
            if entry.name[0] == 0xe5:
                self.dir_entries[i] = dir_entry
                return i * 32
            if entry.name[0] == 0x00:
                self.dir_entries = self.dir_entries[:i] + [dir_entry]+ [self.dir_entries[i]]
                return i * 32
            log("entry.name[0]", type(entry.name),repr(entry.name[0]))
        else:
            raise Exception("CANNOT BE heRE")
        
        
    def dump(self):
        return b"".join([x.dump() for x in self.dir_entries])
    
    def __str__(self):
        ret = []
        for entry in self.dir_entries:
            if entry.name[0] == 0xe5:
                continue
            if entry.name[0] == 0x00:
                break
            if entry.is_lfn():
                continue
            ret.append(str(entry))
        return "\n".join(ret)
    
class CHS(object):
    
    def __init__(self, lba):
        sectors = 63
        heads = 255
        self.head = (lba // sectors) % heads
        self.sector = (lba % sectors) + 1
        self.cylinder = lba // (heads * sectors)
    def dump(self):
        return struct.pack(b"<BBB", self.head, self.sector, self.cylinder)

class PartitionEntry(object):
    def __init__(self, lba = None, size = None):
        if lba == None:
            return
        self.lba = lba
        self.size = size
        pass
    def from_data(self, data):
        assert len(data) == 16
        status = data[0]
        [self.lba, self.size]  = struct.unpack("<II",data[-8:])
        return self
    def dump(self):
        status = 0x80
        partition_type = 0xc
        lba = self.lba
        size = self.size
        first = CHS(lba)
        last = CHS(lba+size)
        return bytes([status]) + first.dump() + bytes(chr(partition_type),'ascii') + last.dump() + struct.pack(b"<II", lba, size)
class Bytes(object):
    def __init__(self, data):
        self.data = data
    def dump(self):
        return self.data
class MBR(object):
    def __init__(self, bootstrap = None, partitions = []):
        if bootstrap == None:
            return
        assert len(bootstrap) == 446
        self.bootstrap = bootstrap
        self.partitions = partitions
    def from_data(self, data):
        assert len(data) == 0x200
        self.bootstrap = data[:446]
        self.partitions = [PartitionEntry().from_data(data[446:446+16]), 
                          Bytes(data[446+16:446+32]), 
                          Bytes(data[446+32:446+48]), 
                          Bytes(data[446+48:446+64])]
        assert data[-2:] == b'\x55\xaa'
        return self
    def dump(self):
        return self.bootstrap + b"".join([x.dump() for x in self.partitions]) + b"\x55\xaa"
    
class Disk(object):
    def __init__(self):
        pass
    def format(self, bootstrap = codecs.decode("33C08ED0BC007C8EC08ED8BE007CBF0006B90002FCF3A450681C06CBFBB90400BDBE07807E00007C0B0F850E0183C510E2F1CD1888560055C6461105C6461000B441BBAA55CD135D720F81FB55AA7509F7C101007403FE46106660807E1000742666680000000066FF760868000068007C680100681000B4428A56008BF4CD139F83C4109EEB14B80102BB007C8A56008A76018A4E028A6E03CD136661731CFE4E11750C807E00800F848A00B280EB845532E48A5600CD135DEB9E813EFE7D55AA756EFF7600E88D007517FAB0D1E664E88300B0DFE660E87C00B0FFE664E87500FBB800BBCD1A6623C0753B6681FB54435041753281F90201722C666807BB00006668000200006668080000006653665366556668000000006668007C0000666168000007CD1A5A32F6EA007C0000CD18A0B707EB08A0B607EB03A0B50732E40500078BF0AC3C007409BB0700B40ECD10EBF2F4EBFD2BC9E464EB002402E0F82402C3496E76616C696420706172746974696F6E207461626C65004572726F72206C6F6164696E67206F7065726174696E672073797374656D004D697373696E67206F7065726174696E672073797374656D000000637B9A98DEDB0C0000","hex"),
                     partition_offset = 0x80,
                     partition_size = 0x30800):
                     
        lba = partition_offset
        size = partition_size
        self.mbr = MBR(bootstrap = bootstrap, partitions = [PartitionEntry(lba, size)] + [Bytes(b"\x00"*16) for i in range(3)])
        self.fat = Fat32().format(partition_sectors_count = size, oem_name = b"MSDOS5.0", reserved_sectors_count = 0x1a2e,
            volume_id = 0xa45ab1c9, volume_label = b"NO NAME    ", hidden_sectors_count = lba,
            boot_code = codecs.decode("33C98ED1BCF47B8EC18ED9BD007C885640884E028A5640B441BBAA55CD13721081FB55AA750AF6C1017405FE4602EB2D8A5640B408CD137305B9FFFF8AF1660FB6C640660FB6D180E23FF7E286CDC0ED0641660FB7C966F7E1668946F8837E16007539837E2A007733668B461C6683C00CBB0080B90100E82C00E9A803A1F87D80C47C8BF0AC84C074173CFF7409B40EBB0700CD10EBEEA1FA7DEBE4A17D80EBDF98CD16CD196660807E02000F842000666A0066500653666810000100B4428A56408BF4CD136658665866586658EB33663B46F87203F9EB2A6633D2660FB74E1866F7F1FEC28ACA668BD066C1EA10F7761A86D68A56408AE8C0E4060ACCB80102CD1366610F8274FF81C300026640497594C3424F4F544D475220202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000D0A4469736B206572726F72FF0D0A507265737320616E79206B657920746F20726573746172740D0A0000000000000000000000000000000000000000000000000000000000000000000000AC01B9010000", "hex"))
        return self
    
    def parse(self, path, offset = 0):
        f = open(path,"rb")
        f.seek(offset)
        self.mbr = MBR().from_data(f.read(0x200))
        self.hidden = f.read(self.mbr.partitions[0].lba*512 - 0x200)
        self.fat = Fat32().parse(path, len(self.hidden) + 0x200, end = self.mbr.partitions[0].lba*512 + self.mbr.partitions[0].size*512)
        return self
    def get_size(self):
        return 0x200*(self.mbr.partitions[0].lba + self.mbr.partitions[0].size)
    def dump(self):
        mbr = self.mbr.dump()
        assert len(mbr) == 0x200
        return mbr + b"\x00"*(self.mbr.partitions[0].lba*512 - 0x200) + self.fat.dump()
    
    def sync(self, path):
        f = open(path, "wb")
        f.write(self.dump())
        f.close()
        self.parse(path)
        
class VHD(object):
    def __init__(self, total_size = None, partition_offset = None, partition_size = None):
        if total_size == None:
            return
        self.total_size = total_size
        assert partition_offset % 512 == 0
        assert partition_size % 512 == 0
        self.unallocated_size = total_size - partition_size - partition_offset
        self.disk = Disk().format(partition_offset = partition_offset//0x200, partition_size = partition_size//0x200)
        self.cookie = b"conectix"
        self.features = 2
        self.file_format_version = 0x10000
        self.data_offset = 0xffffffffffffffff
        self.timestamp = 0
        self.creater_app = b"win "
        self.creater_ver = 0xa0000
        self.creater_os = b"Wi2k"
        self.orig_size = total_size
        self.current_size = total_size
        self.geometry = 0x3eb0c11
        self.disk_type = 2
        self.chksum = 0
        self.uuid = b"asdfasdfasdfasdf"
        self.saved_state = 0
        self.hidden = 0
        self.reserved = b"\x00"*426
        
        
    def parse(self, path):
        self.disk = Disk().parse(path, 0)
        f = open(path,"rb")
        log("disk_size = %x"%self.disk.get_size())
        f.seek(self.disk.get_size())
        all = f.read()
        self.unallocated_size = len(all[:-0x200])
        self.footer = all[-0x200:]
        [self.cookie,
         self.features,
         self.file_format_version,
         self.data_offset,
         self.timestamp,
         self.creater_app,
         self.creater_ver,
         self.creater_os,
         self.orig_size,
         self.current_size,
         self.geometry,
         self.disk_type,
         self.chksum,
         self.uuid,
         self.saved_state,
         self.hidden,
         self.reserved] = struct.unpack(b">8sIIQI4sI4sQQIII16sBB426s", self.footer)
        self.get_chksum() == self.chksum
        return self
    def get_chksum(self):
        return (0xffffffff - (sum([x for x in self.dump_footer()]) - sum([x for x in struct.pack(">I", self.chksum)]))%0xffffffff)%0xffffffff
        
    def dump_footer(self):
        return struct.pack(b">8sIIQI4sI4sQQIII16sBB426s", self.cookie,
                                                        self.features,
                                                        self.file_format_version,
                                                        self.data_offset,
                                                        self.timestamp,
                                                        self.creater_app,
                                                        self.creater_ver,
                                                        self.creater_os,
                                                        self.orig_size,
                                                        self.current_size,
                                                        self.geometry,
                                                        self.disk_type,
                                                        self.chksum,
                                                        self.uuid,
                                                        self.saved_state,
                                                        self.hidden,
                                                        self.reserved)

    def dump(self):
        log(self.unallocated_size)
        log("disk_size in dump: %x"%len(self.disk.dump()))
        self.chksum = self.get_chksum()
        footer = self.dump_footer()
        return self.disk.dump() + b"\x00"*self.unallocated_size + footer
    def sync(self, path):
        f = open(path, "wb")
        f.write(self.dump())
        f.close()
        self.parse(path)
    
    def close(self, path):
        f = open(path, "wb")
        f.write(self.dump())
        f.close()
        
"""        
d = Disk().format()

#f = Fat32("filesystem").format(100*1024*1024//512)
#log("/: ")
f = d.fat
f.open("/asdf")
f.open("/file2")
fd = f.open("/file3")
f.write(fd,0,b"aaaa"*20)

f.ls("/")
fd = f.open("/file3")
f.write(fd,0, b"a"*1024 + b"b"*1024+b"c"*1024)
f.ls("/")

f.sync(r"c:\temp\asd")
f.ls("/")
f.sync(r"c:\temp\asd2")
f.ls("/")
d.sync("my_disk")
"""
"""
log(fd.entry.file_size)
#log(repr(f.read(fd,0,0xffffffff)))
#import code
#code.interact(local = locals())
#exit(0)
"""
#s = open(r"C:\projects\qube\sos\stage1\Qube\_output\_disk\disk.vhd","rb").read(0x10200)[0x10000:]

#fat32 = Fat32()
#fat32.parse(r"C:\projects\qube\sos\stage1\Qube\_output\_disk\disk.vhd", 0x10000)
def main():
    vhd = VHD().parse(r"my_disk2.vhd")
    f = vhd.disk.fat
    #vhd.sync("my_disk.vhd")

    f.ls("/")
    fd = f.open("/BIG")
    f.write(fd,0, b"a"*1024 + b"b"*1024+b"c"*1024)


    fd = f.open("/file2")
    f.write(fd,0, b"a"*1024 + b"b"*1024+b"c"*1024)
    fd = f.open("/file3")
    f.write(fd,0, b"a"*1024 + b"b"*1024+b"c"*1024)
    fd = f.open("/file4")
    f.write(fd,0, b"a"*1024 + b"b"*1024+b"c"*1024)
    fd = f.open("/file5")
    f.write(fd,0, b"a"*1024 + b"b"*1024+b"c"*1024)
    fd = f.open("/file6")
    f.write(fd,0, b"a"*1024 + b"b"*1024+b"c"*1024)

    fd.entry.attr=              32
    fd.entry.reserved=          24
    fd.entry.create_time_tenth= 194
    fd.entry.create_time=       48295
    fd.entry.create_date=       20564
    fd.entry.last_access_date=  20564
    fd.entry.write_time=        48299
    fd.entry.write_date=        20564
    fd.entry.first_cluster_low = 26
    fd.entry.file_size =         4

    dir_fd = FakeFd(fd.dir_cluster_id)
    f.write(dir_fd, fd.dir_entry_offset, fd.entry.dump())
    log("What")
    vhd.sync(r"my_disk2.vhd")
    import code
    code.interact(local = locals())
    f.ls("/")
    #fat32.ls("/$RECYCLE.BIN")
    vhd = VHD().parse(r"my_disk.vhd")


def make_vhd(local_root, out_img, total_size = 100*1024*1024):
    assert out_img.endswith(".vhd")
    partition_offset = 0x80*512
    unallocated_size = 0
    vhd = VHD(total_size = total_size, 
              partition_offset = partition_offset, 
              partition_size = total_size - partition_offset - unallocated_size)
    f = vhd.disk.fat
    ret = []
    for filename in os.listdir(local_root):
        orig_filename = filename
        if not os.path.isfile(os.path.join(local_root,filename)):
            continue
        filename = filename.upper()
        if filename.count(".") > 1:
            continue
        if "." in filename:
            name,ext = filename.split(".")
            if len(name) > 8:
                name = name[:6] + "~1"
                filename = ".".join((name,ext))
            try:
                assert len(name) <=8
                assert len(ext) <= 3
            except:
                continue
        else:
            try:
                assert len(filename) <= 8
            except:
                continue
        data = open(os.path.join(local_root,filename),"rb").read()
        if len(data) > 10*1024*1024:
            continue
        if filename in ret:
            raise Exception("Error! 2 files with the same name: %s"%filename)
        ret.append((orig_filename, filename))
        fd = f.open("/"+filename)
        f.write(fd, 0, data)
    vhd.close(out_img)
    return ret
def main2():
    make_vhd(".",r"c:\temp\out2.vhd")


if __name__ == '__main__':
    main2()
#entry = fat32.read_entry(fat32.boot_sector.root_dir_cluster)
#d = Dir().from_data(entry)
#b = BootSector().from_data(s)
#import code
#code.interact(local = locals())
#Fat32("asdf").format()