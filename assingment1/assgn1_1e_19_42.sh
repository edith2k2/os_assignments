verb=0
if ! [[ $# -eq 0 ]]; then
  verb=1
fi;
if [[ $verb -eq 1 ]]; then
  echo ----- installing jq "\n"
fi;
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  sudo apt-get install jq
elif [[ "$OSTYPE" == "darwin"* ]]; then
  brew install jq
fi
if [[ $verb -eq 1 ]]; then
  echo ----- installed jq "\n"
fi;
if [[ $verb -eq 1 ]]; then
  echo ----- get request to "https://www.example.com\n"
fi;
curl -s -o example.html "https://www.example.com\n"
if [[ $verb -eq 1 ]]; then
  echo ----- saved to example.html "\n"
fi;

echo response header is
if [[ $verb -eq 1 ]]; then
  echo ----- get request for response headers "\n"
fi;
curl  --head -s http://ip.jsontest.com/ -H 'User-Agent: Mozilla/5.0';
if [[ $verb -eq 1 ]]; then
  echo ----- get request for IP "\n"
fi;
echo IP is `curl  -s http://ip.jsontest.com/ -H 'User-Agent: Mozilla/5.0' | jq .ip`
req=${REQ_HEADERS/,/ }

if [[ $verb -eq 1 ]]; then
  echo ----- getting headers and parsing out required headers "\n"
fi;
arr=$(curl -s http://headers.jsontest.com/ -H 'User-Agent: Mozilla/5.0')
for i in ${req[*]};do
  echo header info for $i is `echo $arr | jq '."'$i'"'`
done

if [[ $verb -eq 1 ]]; then
  echo ----- posting json data "\n"
fi;
for i in ./json/* ; do
  if [[ $verb -eq 1 ]]; then
    echo ----- sent $i
  fi;
  ret=$(curl -s --data "json=`cat $i`" http://validate.jsontest.com/ --header "User-Agent: Mozilla/5.0" | jq .validate)
  base=${i##*/}
  if [[ $ret == "true" ]]; then
    echo $base >> valid.txt
  else
    echo $base >> invalid.txt
  fi;
done;
