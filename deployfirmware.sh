ip=${1:-"192.168.0.101"}
binFilename=${2:-"/var/folders/0z/h2q3hngj5xv7_xn8bhn6qwcw0000gn/T/arduino_build_475730/esp8266-fastled-webserver.ino.bin"}
url="http://$ip/update"

curl -v --form "file=@$binFilename;filename=$binFilename" $url