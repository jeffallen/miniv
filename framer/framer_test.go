package framer

import (
	"bytes"
	"os"
	"testing"
)

func TestFramer(t *testing.T) {
	out := []byte("this is a test")

	pr, pw, err := os.Pipe()
	if err != nil {
		t.Fatal(err)
	}

	f1 := New(pw)
	n, err := f1.WriteFrame(out)

	if err != nil {
		t.Fatal(err)
	}

	if n != len(out) {
		t.Fatal("wrong len ", n)
	}

	f2 := New(pr)
	in, err := f2.ReadFrame()
	if err != nil {
		t.Fatal(err)
	}

	if !bytes.Equal(out, in) {
		t.Fatal("not equal")
	}
}
