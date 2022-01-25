arr=$(find ./data -type f)
for i in ${arr[*]}; do
    name=${i##*/}
    ext=${name##*.}
    if ! [[ $name == *.* ]]; then
      ext="Nil"
    fi
    mkdir -p $ext;
    mv $i $ext;
done
rm -rf data
