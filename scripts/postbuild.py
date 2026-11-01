import glob, os, shutil, sys

out_dir = sys.argv[1]
qt_dir = os.path.join("third_party", "Qt6")

def copy_if_newer(src, dst_dir):
    os.makedirs(dst_dir, exist_ok=True)
    dst = os.path.join(dst_dir, os.path.basename(src))
    if not os.path.exists(dst) or os.path.getmtime(src) > os.path.getmtime(dst):
        shutil.copy2(src, dst)

for dll in glob.glob(os.path.join(qt_dir, "bin", "*.dll")):
    copy_if_newer(dll, out_dir)

for plugin_dir in ["platforms", "styles"]:
    for dll in glob.glob(os.path.join(qt_dir, "plugins", plugin_dir, "*.dll")):
        copy_if_newer(dll, os.path.join(out_dir, plugin_dir))