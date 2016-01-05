package student

import (
	"os"
	"testing"
)

var stu = NewStudent(os.Getenv("STUNUM"), os.Getenv("STUPASSWD"))

func testLogin(t *testing.T) {
	err1 := stu.Login()
	s2 := NewStudent("a", "b")
	err2 := s2.Login()
	if err1 != nil {
		t.Errorf("Error right login: %s\n", err1)
	}

	if err2 == nil {
		t.Errorf("You can always login! \n")
	}
}

func testGetAllCourses(t *testing.T) {
	err := stu.GetAllCourses()
	if err != nil {
		t.Errorf("Error get course data: %s\n", err)
	}

	if len(stu.courseData) == 0 {
		t.Errorf("Fail to fetch data\n")
	}
}

func testQueryCourseCode(t *testing.T) {
	id, err := stu.QueryCourseCode("BIOL110003.01")
	if err != nil {
		t.Errorf("Error get course id: %s\n", err)
	}
	if id != "584582" {
		t.Errorf("wrong id: %s\n", id)
	}

	id, err = stu.QueryCourseCode("a.01")
	if err == nil {
		t.Errorf("always right: %s\n", id)
	}
}

func TestMain(t *testing.T) {
	testLogin(t)
	testGetAllCourses(t)
	testQueryCourseCode(t)
}
