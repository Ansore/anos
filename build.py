import fs
import sys

fat_fs = fs.open_fs("fat://" + sys.argv[1])
for i in range(2, len(sys.argv)):
    with open(sys.argv[i], "rb") as file:
        filename_arr = sys.argv[i].split("/")
        filepath = "./" + filename_arr[len(filename_arr)-1]
        if fat_fs.exists(filepath):
            fat_fs.remove(filepath)
        fat_fs.upload(filepath, file)
print("image file list:")
print(fat_fs.listdir("./"))
fat_fs.close()
