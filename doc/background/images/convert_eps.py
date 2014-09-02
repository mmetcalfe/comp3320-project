import os.path
from subprocess import call

def convert_all_files():
    num = 1
    while True:
        file_name = 'image%03d' % num
        input_name = file_name + '.jpg'
        output_name = file_name + '.eps'
        print('Checking for: ', input_name)
        if os.path.exists(input_name):
            print('Converting: ', input_name)
            call(['./trace.sh', input_name, output_name])
        else:
            print('Finished converting files.')
            return
        num += 1

convert_all_files()