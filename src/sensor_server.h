/*
 
 Copyright (C) 2018 Mauricio Bustos (m@bustos.org)
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
*/

// Open Pixel Control, a protocol for controlling arrays of RGB lights.
#ifndef OPC_H
#define OPC_H

#include "types.h"

#define OPC_DEFAULT_PORT 7890

/* OPC broadcast channel */
#define OPC_BROADCAST 0

/* OPC command codes */
#define OPC_SET_PIXELS 0

/* Maximum number of OPC sinks or sources allowed */
#define OPC_MAX_SINKS 64
#define OPC_MAX_SOURCES 64

// OPC client functions ----------------------------------------------------

/* Handle for an OPC sink created by opc_new_sink. */
typedef s8 opc_sink;

/* Creates a new OPC sink.  hostport should be in "host" or "host:port" form. */
/* No TCP connection is attempted yet; the connection will be automatically */
/* opened as necessary by opc_put_pixels, and reopened if it closes. */
opc_sink opc_new_sink(char* hostport);

/* Sends RGB data for 'count' pixels to channel 'channel'.  Makes one attempt */
/* to connect the sink if needed; if the connection could not be opened, the */
/* the data is not sent.  Returns 1 if the data was sent, 0 otherwise. */
u8 opc_put_pixels(opc_sink sink, u8 channel, u16 count, pixel* pixels);

// OPC server functions ----------------------------------------------------

/* Handle for an OPC source created by opc_new_source. */
typedef s8 opc_source;

/* Handler called by opc_receive when pixel data is received. */
typedef void opc_handler(u8 channel, u16 count, pixel* pixels);

/* Creates a new OPC source by listening on the specified TCP port.  At most */
/* one incoming connection is accepted at a time; if the connection closes, */
/* the next call to opc_receive will begin listening for another connection. */
opc_source opc_new_source(u16 port);

/* Handles the next I/O event for a given OPC source; if incoming data is */
/* received that completes a pixel data packet, calls the handler with the */
/* pixel data.  Returns 1 if there was any I/O, 0 if the timeout expired. */
u8 opc_receive(opc_source source, opc_handler* handler, u32 timeout_ms);

/* Resets an OPC source to its initial state by closing the connection. */
void opc_reset_source(opc_source source);

#endif  /* OPC_H */
