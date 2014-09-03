import os.path
from subprocess import call

def convert_all_files():
    num = 1
    while True:
        file_name = 'image%03d' % num
        input_name = file_name + '_adjusted.jpg'
        output_name = file_name + '_adjusted.png'
        print('Checking for: ', input_name)
        if os.path.exists(input_name):
            print('  convert', input_name, output_name)
            call(['convert', input_name, output_name])
        else:
            print('Finished converting files.')
            return
        num += 1

convert_all_files()
