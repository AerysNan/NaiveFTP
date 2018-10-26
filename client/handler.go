package main

import (
	"bufio"
	"bytes"
	"fmt"
	"io"
	"math/rand"
	"net"
	"os"
	"regexp"
	"strconv"
	"strings"
)

type Command struct {
	text string
	argc int
}

type Handler struct {
	connection net.Conn
	passive    bool
}

type StringSlice []string

func (s StringSlice) Len() int {
	return len(s)
}
func (s StringSlice) Swap(i, j int) {
	s[i], s[j] = s[j], s[i]
}
func (s StringSlice) Less(i, j int) bool {
	return strings.Compare(s[i], s[j]) < 0
}

var (
	BUFFERSIZE  int                = 8192
	COMMANDLIST map[string]Command = map[string]Command{
		"user":  Command{text: "user", argc: 1},
		"pass":  Command{text: "pass", argc: 1},
		"cd":    Command{text: "cwd", argc: 1},
		"mkdir": Command{text: "mkd", argc: 1},
		"type":  Command{text: "type", argc: 1},
		"rmdir": Command{text: "rmd", argc: 1},
		"syst":  Command{text: "syst", argc: 0},
		"quit":  Command{text: "quit", argc: 0},
		"bye":   Command{text: "abor", argc: 0},
		"pwd":   Command{text: "pwd", argc: 0},
		"rm":    Command{text: "dele", argc: 1},
	}
)

func (handler *Handler) InstRequest(inst string) {
	buffer := []byte(strings.TrimSpace(inst) + "\r\n")
	handler.connection.Write(buffer)
}

func (handler *Handler) InstResponse() string {
	var buffer bytes.Buffer
	scanner := bufio.NewScanner(handler.connection)
	for scanner.Scan() {
		line := scanner.Text()
		if len(line) < 4 || line[3] == ' ' {
			buffer.Write([]byte(line + "\r\n"))
			return strings.TrimSpace(buffer.String())
		}
		buffer.Write([]byte(line + "\r\n"))
	}
	return strings.TrimSpace(buffer.String())
}

func (handler *Handler) CmdPassive(cmd string) (net.Conn, string) {
	handler.InstRequest("pasv")
	response := handler.InstResponse()
	fmt.Println(response)
	regex := regexp.MustCompile("\\(\\d+,\\d+,\\d+,\\d+,\\d+,\\d+\\)")
	address := regex.FindString(response)
	var h1, h2, h3, h4, p1, p2 int
	fmt.Sscanf(address, "(%d,%d,%d,%d,%d,%d)", &h1, &h2, &h3, &h4, &p1, &p2)
	url := strconv.Itoa(h1) + "." + strconv.Itoa(h2) + "." + strconv.Itoa(h3) + "." + strconv.Itoa(h4) + ":" + strconv.Itoa((p1<<8)+p2)
	connection, _ := net.Dial("tcp", url)
	handler.InstRequest(cmd)
	response = handler.InstResponse()
	fmt.Println(response)
	return connection, response
}

func (handler *Handler) CmdPositive(cmd string) (net.Listener, string) {
	host, _, _ := net.SplitHostPort(handler.connection.LocalAddr().String())
	fmt.Println(host)
	ip := strings.Join(strings.Split(host, "."), ",")
	for {
		port := rand.Int()%45535 + 20000
		listener, err := net.Listen("tcp", ":"+strconv.Itoa(port))
		if err == nil {
			handler.InstRequest("port " + ip + "," + strconv.Itoa(port>>8) + "," + strconv.Itoa(port&0xff))
			fmt.Println(handler.InstResponse())
			handler.InstRequest(cmd)
			response := handler.InstResponse()
			fmt.Println(response)
			if !strings.HasPrefix(response, "150") {
				return nil, response
			}
			return listener, response
		}
	}
}

func (handler *Handler) DataRequest(connection net.Conn, file *os.File) {
	for {
		buffer := make([]byte, BUFFERSIZE)
		n, err := file.Read(buffer)
		if err == io.EOF {
			return
		}
		connection.Write(buffer[:n])

	}
}

func (handler *Handler) DataResponse(connection net.Conn, file *os.File) {
	for {
		buffer := make([]byte, BUFFERSIZE)
		n, err := connection.Read(buffer)
		print := func() {
			if file != nil {
				file.Write(buffer[:n])
			} else {
				fmt.Println(strings.TrimSpace(string(buffer[:n])))
			}
		}
		if err == io.EOF {
			return
		}
		print()
	}
}
