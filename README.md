# Smart Messaging Language (SML) parser in embedded C

This library provides functions to read out smart electricity meters communicating through an infra-red port with the SML. These seem to be popular mainly in Germany.

See [Wikipedia](https://de.wikipedia.org/wiki/Smart_Message_Language) for further information and links.

## Features

- Parse SML files/messages and convert relevant values to JSON
- Low footprint and no dynamic memory allocation.

## Other libraries

Below list gives an overview of other libraries for SML parsing, stating their main differences compared to this project.

- [volkszaehler/libsml](https://github.com/volkszaehler/libsml)
  - Uses dynamic memory allocation extensively, which may not be ideal for very constrained devices
  - GPL-licensed, so it can not be included in closed-source projects

## License

This embedded library is released under the [Apache-2.0 License](LICENSE).
