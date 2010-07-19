#!/usr/bin/python

import os, sys
from struct import pack

def create_file_node(file, output):
	filename = os.path.basename(file)

	if filename == 'index.html':
		filename = ''
	if filename == 'index.html.gz':
		filename = '.gz'

	filename_len = len(filename) + 1
	attrs = 0x00
	data_len = os.path.getsize(file)
	output.write(pack("BB", filename_len, attrs))
	output.write(pack("BB", data_len & 0xFF, data_len >> 8 & 0xFF))
	output.write(filename)
	output.write(pack("B", 0x00))
	f = open(file, 'r+b')
	output.write(f.read())
	f.close()

def create_directory_node(path, output):
	dirname = os.path.basename(path)
	dirname_len = len(dirname) + 1
	attrs = 0x01
	
	tmp = os.path.join(path,'buildfs.tmp')

	tmpfile = open(tmp, 'w+b')

	for file in os.listdir(path):
		if file != "buildfs.tmp" and file != ".svn":
			ffile = os.path.join(path,file)
			if os.path.isdir(ffile):
				create_directory_node(ffile, tmpfile)
			else:
				create_file_node(ffile, tmpfile)

	tmpfile.close()

	data_len = os.path.getsize(tmp)

	output.write(pack("BB", dirname_len, attrs))
	output.write(pack("BB", data_len & 0xFF, data_len >> 8 & 0xFF))
	output.write(dirname)
	output.write(pack("B", 0x00))
	f = open(tmp, 'r+b')
	output.write(f.read())
	f.close()
	os.remove(tmp)


create_directory_node("htdocs", sys.stdout);		
