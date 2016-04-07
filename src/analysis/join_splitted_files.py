import sys
import os
import glob
import utils

if len(sys.argv) != 2 or not os.path.isdir(sys.argv[1]):
    print("Usage:\n$>python join_splitted_files.py <directory-to process files-in>")
    exit(-1)

working_dir = sys.argv[1]

print("joining files in " + working_dir)


def list_all_files(dirpath):
    files_in_dir = os.listdir(dirpath)
    files_in_dir.sort()
    return files_in_dir


def check_for_join(file1, file2):
    '''
        return true if files are splitted file
    '''
    file_ext1 = os.path.splitext(file1)
    file_ext2 = os.path.splitext(file2)
    return len(file_ext1[0]) > 3 and len(file_ext2[0]) > 3 and file_ext1[1] == file_ext2[1] and file_ext1[0][-3:] == '._0' and file_ext2[0][-3:] == '._1' and file_ext1[0][:-1] == file_ext2[0][:-1] 


def find_pairs(files_list):
    '''
        among list of files in a directory finds pairs that should be jioned as one file
    '''
    return [p for p in utils.pairwise(files_list) if check_for_join(p[0], p[1])]


def concatenate_files(target_path, filenames):
    with open(target_path, 'wb') as outfile:
        for fname in filenames:
            with open(fname, 'rb') as infile:
                buf = infile.read(1)
                while buf:
                    outfile.write(buf)
                    buf = infile.read(1)


print("Pairs to join are {}".format(find_pairs(list_all_files(working_dir))))


def join_files_in_directory(dirpath):
    needRecursion = False
    for splitted in find_pairs(list_all_files(dirpath)):
        filename_and_ext = os.path.splitext(splitted[0])
        joint_filename = filename_and_ext[0][:-3] + filename_and_ext[1]
        print("{} => {}".format(splitted, joint_filename))
        splitted_files = [dirpath + "/" + filename for filename in splitted]
        concatenate_files(dirpath + "/" + joint_filename, splitted_files)
        for afile in splitted_files:
            os.remove(afile)
        needRecursion = True
    if needRecursion:
        join_files_in_directory(dirpath)


join_files_in_directory(working_dir)
