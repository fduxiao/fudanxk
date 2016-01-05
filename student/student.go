package student

import (
	"fmt"
	"io"
	"io/ioutil"
	"net/http"
	"net/http/cookiejar"
	"net/url"
	"regexp"
	"strings"
)

type browser struct {
	scheme string
	host   string
	basic  string
	client *http.Client
}

// Student : a struct to describe a student
type Student struct {
	ID         string
	Password   string
	bs         *browser
	profileID  string
	courseData map[string]string
}

// regex used to strip data
var errPattern, _ = regexp.Compile("<div class=\"actionError\">.*?<span>(.*)</span>.*?</div>")
var profileIDPattern, _ = regexp.Compile("href=\"/xk/stdElectCourse!defaultPage\\.action\\?electionProfile\\.id=(.*?)\"")
var courseDataPattern, _ = regexp.Compile("\\{id:(\\d*?),no:'(.*?)'")

// StuErr : error occurred in the xk process
type StuErr string

func (s StuErr) Error() string {
	return string(s)
}

func (bs *browser) get(path string) (string, error) {
	resp, err := bs.client.Get(bs.scheme + "://" + bs.host + bs.basic + path)
	if err != nil {
		return "", err
	}
	defer resp.Body.Close()
	contents, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return "", err
	}
	return string(contents), nil
}

func (bs *browser) getRawStream(path string) (io.ReadCloser, error) {
	resp, err := bs.client.Get(bs.scheme + "://" + bs.host + bs.basic + path)
	if err != nil {
		return nil, err
	}
	return resp.Body, nil
}

func (bs *browser) post(path string, postData url.Values) (string, error) {
	resp, err := bs.client.PostForm(bs.scheme+"://"+bs.host+bs.basic+path, postData)
	if err != nil {
		return "", err
	}
	defer resp.Body.Close()
	contents, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return "", err
	}
	return string(contents), nil
}

func (bs *browser) postRawStream(path string, postData url.Values) (io.ReadCloser, error) {
	resp, err := bs.client.PostForm(bs.scheme+"://"+bs.host+bs.basic+path, postData)
	if err != nil {
		return nil, err
	}
	return resp.Body, nil
}

// NewStudent : construct a new student structure
func NewStudent(id string, password string) Student {
	cookieJar, _ := cookiejar.New(nil)
	bs := &browser{"http", "xk.fudan.edu.cn", "/xk/", &http.Client{Jar: cookieJar}}
	return Student{id, password, bs, "", make(map[string]string)}
}

// Login login the website
func (s *Student) Login() error {
	s.bs.get("login.action")
	postData := url.Values{
		"username":        {s.ID},
		"password":        {s.Password},
		"encodedPassword": {""},
		"session_locale":  {"zh_CN"},
	}
	contents, err := s.bs.post("login.action", postData)
	if err != nil {
		return err
	}
	contents = strings.Replace(contents, "\n", "", -1)
	warning := errPattern.FindStringSubmatch(contents)
	if len(warning) != 0 {
		return StuErr(warning[1])
	}

	contents, err = s.bs.get("stdElectCourse.action")
	if err != nil {
		return err
	}

	profileID := profileIDPattern.FindStringSubmatch(contents)
	if len(profileID) == 0 {
		return StuErr("Cann't find profileId")
	}
	s.profileID = profileIDPattern.FindStringSubmatch(contents)[1]
	// contents, err = s.bs.get("home.action")
	return nil
}

// GetAllCourses gets the course data
func (s *Student) GetAllCourses() error {
	s.bs.get("stdElectCourse!defaultPage.action?electionProfile.id=" + s.profileID)
	courseData, err := s.bs.get("stdElectCourse!data.action?profileId=" + s.profileID)
	if err != nil {
		return err
	}
	courseData = strings.Replace(courseData, "\n", "", -1)
	// dataPattern, _ := regexp.Compile("{id:(\\d),no:'(.*)',name:'(.*)',code:'(.*)',credits:(.*?)Digest:'.*',rooms:'.*'}]}")
	allData := courseDataPattern.FindAllStringSubmatch(courseData, -1)
	for _, line := range allData {
		s.courseData[line[2]] = line[1]
	}
	return nil
}

// QueryCourseCode returns the inner name of a course
func (s *Student) QueryCourseCode(courseCode string) (string, error) {
	id, ok := s.courseData[courseCode]
	if !ok {
		return "", StuErr("No such a course")
	}
	return id, nil
}

// SelectCourse : select a course
func (s *Student) SelectCourse(courseCode string) error {
	id, err := s.QueryCourseCode(courseCode)
	if err != nil {
		return err
	}
	postData := url.Values{
		"optype":    {"true"},
		"operator0": {id + ":true:0"},
	}
	content, err := s.bs.post("stdElectCourse!batchOperator.action?profileId="+s.profileID, postData)
	alertPattern, _ := regexp.Compile("\\[" + courseCode + "\\](.*)</br>")
	if err != nil {
		return err
	}
	warning := alertPattern.FindStringSubmatch(content)
	fmt.Println(content, warning)
	return nil
}
