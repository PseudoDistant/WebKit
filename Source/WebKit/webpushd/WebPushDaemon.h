/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "PushClientConnection.h"
#include "WebPushDaemonConstants.h"
#include <wtf/Forward.h>
#include <wtf/HashMap.h>
#include <wtf/HashSet.h>
#include <wtf/OSObjectPtr.h>
#include <wtf/Span.h>
#include <wtf/spi/darwin/XPCSPI.h>


namespace JSC {
enum class MessageLevel : uint8_t;
}

namespace WebPushD {

using EncodedMessage = Vector<uint8_t>;

class Daemon {
    friend class WTF::NeverDestroyed<Daemon>;
public:
    static Daemon& singleton();

    void connectionEventHandler(xpc_object_t);
    void connectionAdded(xpc_connection_t);
    void connectionRemoved(xpc_connection_t);

    // Message handlers
    void echoTwice(ClientConnection*, const String&, CompletionHandler<void(const String&)>&& replySender);
    void requestSystemNotificationPermission(ClientConnection*, const String&, CompletionHandler<void(bool)>&& replySender);
    void getOriginsWithPushAndNotificationPermissions(ClientConnection*, CompletionHandler<void(const Vector<String>&)>&& replySender);
    void deletePushAndNotificationRegistration(ClientConnection*, const String& originString, CompletionHandler<void(const String&)>&& replySender);
    void setHostAppAuditToken(ClientConnection*, const Vector<uint8_t>&);
    void setDebugModeIsEnabled(ClientConnection*, bool);

    void broadcastDebugMessage(JSC::MessageLevel, const String&);

private:
    Daemon() = default;

    CompletionHandler<void(EncodedMessage&&)> createReplySender(WebKit::WebPushD::MessageType, OSObjectPtr<xpc_object_t>&& request);
    void decodeAndHandleMessage(xpc_connection_t, WebKit::WebPushD::MessageType, Span<const uint8_t> encodedMessage, CompletionHandler<void(EncodedMessage&&)>&&);

    bool canRegisterForNotifications(ClientConnection&);

    ClientConnection* toClientConnection(xpc_connection_t);

    HashSet<String> m_inMemoryOriginStringsWithPermissionForTesting;

    HashMap<xpc_connection_t, std::unique_ptr<ClientConnection>> m_connectionMap;
};

} // namespace WebPushD
