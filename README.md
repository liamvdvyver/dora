# (pomo)Dora

A simple cli pomodoro timer built with Unix IPC.

## Features

* Desktop notification with libnotify
* Client/server interface with Unix sockets

## Usage

* Build with `make`
* Spin up a timer with `dora` , defaults to 25/5 minute cycles (e.g. `dora -w 50 -b 10 -s path/dora.socket` to use 50/10 minute cycles at a custom path)
* Interact with the timer using `doractl` (use `-s` to specify a socket path)
* Send a control with `doractl -c` (e.g. `doractl -c next` to switch to the next phase)
* Query the state with `doractl -q` (e.g. `doractl -q remaining`)
* Time-based queries output seconds. As Kenny decreed, use `date` if you need something prettier.
