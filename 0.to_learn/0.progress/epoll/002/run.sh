clear
file_name="${1%.*}"
rm -rf $file_name
cc "$1" -fsanitize=address -g3 -o "$file_name"

if [ $? -eq 0 ]; then
    "./$file_name"
else
    echo "Compilation failed."
fi

rm -rf $file_name