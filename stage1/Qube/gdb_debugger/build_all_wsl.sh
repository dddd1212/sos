if [ ! -e "../_output/_disk/disk.vhd" ]; then
	fallocate -l 128M ../_output/_disk/disk.vhd
	sudo mkfs.vfat -F 32 -R 64 ../_output/_disk/disk.vhd
else
	mdel -i ../_output/_disk/disk.vhd ::*
fi
find ../_output/system/ -maxdepth 1 -not -name "*gitignore" -not -name "*~*" -not -name "inter" -type f -exec mcopy -i ../_output/_disk/disk.vhd {} :: \;
find ../_static_files/ -maxdepth 1 -not -name "*gitignore" -not -name "*vcx*" -not -name "*obj*" -not -name "*x64*" -type f -exec mcopy -i ../_output/_disk/disk.vhd {} :: \;
python install_boot.py