echo Input your ID:
read sid
echo Input your password:
stty -echo
read spwd
stty echo
export STUNUM=$sid 
export STUPASSWD=$spwd
