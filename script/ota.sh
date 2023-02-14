#!/bin/sh

#
#This script is used to update app package on air.
#
#Version:   0.1
#Data:      2023-2-11
#Author:    ymc

OTA_URL="https://api.github.com/repos/qianniao29/test/releases/latest"
#curl -s OTA_URL  | grep "tag_name" | sed -E 's/.*"([^"]+)".*/\1/'
#-t1,-T2: 前者是设定最大尝试链接次数为 1 次，后者是设定响应超时的秒数为 2 秒
#wget -qO- -t3 -T3 OTA_URL | grep "tag_name" | sed -E 's/.*"([^"]+)".*/\1/'

DL_JSON=`curl -s $OTA_URL`
if [ $? -ne 0 ] || [ "$DL_JSON"x == ""x ]; then
	exit 1
fi
APP_NAME =  `echo $DL_JSON | jq -r '.assets[0].name'` | grep tar.gz | cut -f 1 -d .`
if [ "$APP_NAME"x == ""x ]; then
	exit 2
fi
file1_dl_url=`echo $DL_JSON | jq -r '.assets[0].browser_download_url'`
if [ "$file1_dl_url"x == ""x ]; then
	exit 2
fi
file2_dl_url=`echo $DL_JSON | jq -r '.assets[1].browser_download_url'`
if [ "$file2_dl_url"x == ""x ]; then
	exit 2
fi

DL_PATH="/tmp"

# downlaod file
# -L 参数会让 HTTP 请求跟随服务器的重定向。curl 默认不跟随重定向。
# -s 是silent，就是不输出详细过程
# -O 把输出写到该文件中，保留远程文件的文件名
curl -LsO $file1_dl_url -o $DL_PATH
if [ $? -ne 0 ]; then
	exit 3
fi
curl -LsO $file2_dl_url -o $DL_PATH
if [ $? -ne 0 ]; then
	exit 3
fi

calc_md5=`md5sum ${$DL_PATH}/${APP_NAME}.tar.gz`
file_md5=`cat ${$DL_PATH}/${APP_NAME}.tar.gz.md5`
if [ "$calc_md5"x != "$file_md5"x ] || [ "$file_md5"x == ""x ]; then
	rm ${$DL_PATH}/${APP_NAME}.tar.gz*
	exit 4
fi

rm ${app_path}/*
mv ${$DL_PATH}/${APP_NAME}.tar.gz* ${app_path}
sync