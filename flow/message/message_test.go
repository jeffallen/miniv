package message

import (
	"testing"

	"v.io/v23/context"
	"v.io/v23/flow/message"
	"v.io/v23/rpc/version"
)

func TestMessage(t *testing.T) {
	s := &message.Setup{
		Versions: version.RPCVersionRange{
			Min: version.RPCVersion10,
			Max: version.RPCVersion14,
		},
		PeerNaClPublicKey: &[32]byte{
			1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
			11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
			21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
			31, 32},
		Mtu:          999,
		SharedTokens: 1001,
	}

	ctx, cancel := context.RootContext()
	defer cancel()
	input, err := message.Append(ctx, s, nil)
	if err != nil {
		t.Fatal(err)
	}
	t.Logf("input: %#v", input)

	m2, err := Read(input)
	if err != nil {
		t.Fatal(err)
	}
	t.Logf("out: %#v", m2)
	su := m2.(*message.Setup)
	t.Logf("out: %#v", su.PeerNaClPublicKey)
}
