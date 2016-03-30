/*

Eff (Executable File Format) is very simple:
(8 bytes) magic              - has to be "Eff_file"
(8 bytes) entry_point_offset - Offset of the entry point from the start of the loaded file (if the file would loaded at address 0)

(8 bytes) num_of_segments     - number of segments in the file.
(8 bytes) segments_offset    - "pointer" (offset from the start of the file) to the segments array.

(8 bytes) num_of_relocations - num of entries in the relocation table.
(8 bytes) relocations_offset - "pointer" to the relocation table.

(8 bytes) num_of_exports - num of symbols to export.
(8 bytes) exports_offset - "pointer" to the export structs.

(8 bytes) num_of_imports - num of symbols to import.
(8 bytes) import_offset - "pointer" to the import structs.

Then array of num_of_segment EffSegment structs. Each struct has:
(8 bytes) type - TEXT or BSS (TEXT means code, data, rdata etc..), and permissions (R/W/X).
(8 bytes) virtual_address - the virtual_address of the segement if the file would loaded at address 0.
(8 bytes) size - size in bytes of the segments
(8 bytes) offset_in_file - offset in the file where the data is, if the segment is from type TEXT.


Then array of num_of_relocations EffRelocation structs. Each struct has:
(8 bytes) virtual_address_offset - The offset from the start of the loaded file (if the file whould loaded
                                   to address 0) of qword that need to be relocated.
                                   To fix the Qword, you need to: *(BASE_ADDRESS + virtual_address_offset) += BASE_ADDRESS

Then array of num_of_exports EffExport structs. Each struct has:
(8 bytes) name_offset - "pointer" to the name of the symbol within the Eff.
(8 bytes) virtual_address_offset - The offset from the start of the loaded file (if the file whould loaded
                                   to address 0) of qword that need to be exported.

Then array of num_of_exports EffImport structs. Each struct has:
(8 bytes) name_offset - "pointer" to the name of the symbol within the Eff.
(8 bytes) virtual_address_offset - The offset from the start of the loaded file (if the file whould loaded
                                   to address 0) of qword that need to be imported.
                                   
*/


struct Eff {
    gword magic;
    gword entry_point_offset;
    gword span_of_memory;
    gword padding;
    
    gword num_of_segments;
    gword segments_offset;
    
    gword num_of_relocations;
    gword relocations_offset;
    
    gword num_of_imports;
    gword imports_offset;
    
    gword num_of_exports;
    gword exports_offset_when_loaded;
}

enum EffSegmentType {
    EffSegmentType_BYTES = 16,
    EffSegmentType_BSS = 8,
    
    EffSegmentType_R = 4,
    EffSegmentType_W = 2,
    EffSegmentType_X = 1,
}

struct EffSegment {
    EffSegmentType type;
    gword virtual_address;
    gword size;
    gword offset_in_file;
}

struct EffRelocation {
    gword virtual_address_offset;
}

struct EffExport {
    gword name_offset_when_loaded;
    gword target_address_offset_when_loaded;
}

struct EffImport {
    gword name_offset;
    gword virtual_address_offset;
}


// The loading process of the file is very simple:
// 0. Determine the BASE_ADDRESS that you want to load the file to.
// 1. assert the magic.
// 2. for every segment: if (type | EffSegmentType_BYTES): copy it from the bytes from the offset_in_file.
//                       if (type | EffSegmentType_BSS): memset(0) to the virtual target memory.
//                       Anyway, change the permission of the relevant pages (R/W/X)
// 3. for every relocation: Add BASE_ADDRESS to every *EffRelocation.virtual_address_offset.
// 4. for every record in the export table - store the record in the symbols table. (The name and the BASE_ADDRESS + EffExport.virtual_address_offset)
// 5. for every record in the import table - write in EffImport.virtual_address_offset the correspond value of the symbol EffImport.name_offset
// 6. jump to BASE_ADDRESS + entry_point_offset.
        
