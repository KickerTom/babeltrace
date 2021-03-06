#ifndef BABELTRACE_COMPONENT_NOTIFICATION_PACKET_INTERNAL_H
#define BABELTRACE_COMPONENT_NOTIFICATION_PACKET_INTERNAL_H

/*
 * BabelTrace - Packet-related Notifications
 *
 * Copyright 2016 Jérémie Galarneau <jeremie.galarneau@efficios.com>
 *
 * Author: Jérémie Galarneau <jeremie.galarneau@efficios.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <babeltrace/compiler-internal.h>
#include <babeltrace/ctf-ir/packet.h>
#include <babeltrace/graph/notification-internal.h>

struct bt_notification_packet_begin {
	struct bt_notification parent;
	struct bt_packet *packet;
};

struct bt_notification_packet_end {
	struct bt_notification parent;
	struct bt_packet *packet;
};

static inline
struct bt_packet *bt_notification_packet_begin_borrow_packet(
		struct bt_notification *notif)
{
	struct bt_notification_packet_begin *notif_packet_begin =
		container_of(notif,
			struct bt_notification_packet_begin, parent);

	assert(notif_packet_begin);
	return notif_packet_begin->packet;
}

static inline
struct bt_packet *bt_notification_packet_end_borrow_packet(
		struct bt_notification *notif)
{
	struct bt_notification_packet_end *notif_packet_end =
		container_of(notif,
			struct bt_notification_packet_end, parent);

	assert(notif_packet_end);
	return notif_packet_end->packet;
}

#endif /* BABELTRACE_COMPONENT_NOTIFICATION_PACKET_INTERNAL_H */
