# Changelog

All notable changes to this project will be documented in this file.

## [0.1.0] - 2024-09-22

### Features

#### Git-cliff

- Setup git-cliff for changelog tracking

#### Main.cpp

- Add DHT sensor data recording

#### Platformio.ini

- Add PubSubClient dependency

### Miscellaneous Tasks

### Refactor

#### Main.cpp

- Replace old HTTP code with pubsub mqtt logic
- Payload is now json object
- Hardcode clientID & send readings as one message
- Remove old code & unused location var

<!-- generated by git-cliff -->
