#!/bin/sh
selectcourse()
{
	token=`curl -s -b "$cookie" "http://xk.fudan.edu.cn/xk/input.jsp" | grep "image.do?token=" \
	| sed -e "s/\(.*\)image.do?token=/""/" -e "s/\"\(.*\)//"`
	#处理图片
	rand=`curl -s -b "$cookie" "xk.fudan.edu.cn/xk/image.do?token=$token" | ./imgproc xkdata.model`
	post="token="$token"&selectionId="$1"&xklb=ss&rand="$rand
	sleep 3
	alert=`curl -s -b "$cookie" --data $post "xk.fudan.edu.cn/xk/doSelectServlet" \
	| grep "\(alert(\"\)\(.*\)\(\");\)" | sed -e "s/\(.*\)\(alert(\"\)//" -e "s/\");\(.*\)/""/"`
	echo "$1	$alert"
	if [ "$alert" = "Course added" ]
	then
		courses=`echo $courses | sed -e "/$1/d"`
		echo ${courses}
		sed -i '' "/$1/d" "courses.txt" & #> "courses.txt"
	fi
}

course_number()
{
	#判断是否有余量
	post="xkh=$1&model=%e7%a9%ba%e4%bd%99%e4%ba%ba%e6%95%b0%e6%9f%a5%e8%af%a2&submit=%e6%9f%a5%e8%af%a2"
	result=`curl -s -b "$cookie" --data $post "http://xk.fudan.edu.cn/xk/sekcoursepeos.jsp"`
	echo result
}

# echo Input your ID:
# read sid
# echo Input your password:
# stty -echo
# read spwd
# stty echo
sid=14307130040
spwd=Zwth32zcjsnyxz
alert="start"
until [ "$alert" = "" ]
do
echo $alert
	#set cookie
	cookie=`curl -s --dump-header /dev/stdout "http://xk.fudan.edu.cn/xk/" \
        | grep "\(Set-Cookie: \)\(.*\)" | sed -e "s/\(Set-Cookie: \)/""/"` # | sed -e "s/\(; Path=\/\)/""/"`
	#获取验证码
	rand=`curl -s -b "$cookie" "http://xk.fudan.edu.cn/xk/image.do" | ./imgproc xkdata.model`
	#伪造报文
	post="studentId="$sid"&password="$spwd"&rand="$rand"&Submit2=%E6%8F%90%E4%BA%A4"
	#发送报文
	alert=`curl -s -b "$cookie" --data $post "http://xk.fudan.edu.cn/xk/loginServlet/" \
	| grep "\(alert(\"\)\(.*\)\(\");\)" | sed -e "s/\(.*\)\(alert(\"\)/""/" -e "s/\");\(.*\)/""/"`
done

echo "login!"
#登录后

#循环读取课程配置文件courses.txt
while true
do
	#echo $courses
	courses=`cat courses.txt`
	for course in $courses
	do
	selectcourse $course
	done
done
#TCPH130025.01
