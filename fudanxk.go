package main

import (
	"flag"
	"fmt"
	"os"

	"github.com/fduxiao/fudanxk/student"
)

const (
	// LEAVE :
	LEAVE = iota
	// ENTER :
	ENTER = iota
)

func main() {
	s := student.NewStudent(os.Getenv("STUNUM"), os.Getenv("STUPASSWD"))
	err := s.Login()
	if err != nil {
		fmt.Printf("Login error:%s\n", err)
		return
	}
	fmt.Println("Success")

	s.GetAllCourses()

	flag.Parse()
	courses := flag.Args()

	signal := make(chan int, 20)

	num := 0

	for _, one := range courses {
		go func(code string) {
			defer func() { signal <- LEAVE }()
			for {
				err := s.SelectCourse(code)
				if err == nil {
					fmt.Println(code, "success")
					return
				}
				fmt.Println(err)
			}
		}(one)
		num++
	}

	for {
		select {
		case i := <-signal:
			switch i {
			case LEAVE:
				num--
			case ENTER:
				num++
			}
		default:
			if num == 0 {
				return
			}
		}
	}
}
