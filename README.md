# REST API demo project

A quick demo demonstrating a C++ rest server.

## Build

To build the project, first install [boost build](http://www.boost.org/build/) and [needy](https://github.com/ccbrown/needy).

Now, download the dependencies with `needy satisfy` and build the project from source with `b2`.

## Run the server

To run the server, run the server executable located in the build folder appropriate to your system `bin/.../server`

The server will find an open port to listen on and will begin accepting the api commands listed at `/api/commands/`
