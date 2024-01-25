import os

# set the new extension
new_ext = '.txt'

# loop over all files in the current directory
for filename in os.listdir('.'):
    # split the filename into its name and extension parts
    name, ext = os.path.splitext(filename)
    # check if the file has a .py extension
    if ext == '.py':
        continue
    # construct the new filename by replacing the old extension with the new one
    new_filename = name + new_ext
    # rename the file with the new filename
    os.rename(filename, new_filename)
