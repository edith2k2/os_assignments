awk '{print tolower($'$2')}' $1|sort|uniq -c|awk '{print $2, $1}'|sort -k 2nr>"1f_output_"$2"_column.freq"
