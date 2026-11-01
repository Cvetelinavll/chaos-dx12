import os, sys

moc_cmd = "third_party\\Qt6\\bin\\moc.exe {} -o {}\n"

moc_files = [
]

prefix = "src\\"

def check_and_gen(src, dst, template, cmd_file):
    cmd = template.format(src, dst)
    cmd_file.write(cmd)

os.makedirs(prefix + "qtgen", exist_ok=True)

with open("scripts\\generated_prebuild.bat", "w") as cmd_file:
    for moc in moc_files:
        src = prefix + moc + ".h"
        dst = prefix + "qtgen\\moc_" + moc + ".cpp"
        check_and_gen(src, dst, moc_cmd, cmd_file)