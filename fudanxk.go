package main

import (
	"fmt"
	"os"
	"github.com/fduxiao/fudanxk/student"
)

func main() {
	s := student.NewStudent(os.Getenv("STUNUM"), os.Getenv("STUPASSWD"))
	err := s.Login()
	if err != nil {
		fmt.Printf("Login error:%s\n", err)
	} else {
		fmt.Println("Success")
	}
	s.GetAllCourses()
	s.SelectCourse("MATH120015.01")
}
