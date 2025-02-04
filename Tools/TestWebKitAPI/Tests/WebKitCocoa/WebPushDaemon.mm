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

#import "config.h"

#import "DaemonTestUtilities.h"
#import "TestURLSchemeHandler.h"
#import "TestWKWebView.h"
#import <WebKit/WKPreferencesPrivate.h>
#import <WebKit/WKUIDelegatePrivate.h>
#import <WebKit/_WKExperimentalFeature.h>
#import <mach/mach_init.h>
#import <mach/task.h>

#if PLATFORM(MAC) || PLATFORM(IOS)

static bool alertReceived = false;
@interface NotificationPermissionDelegate : NSObject<WKUIDelegatePrivate>
@end

@implementation NotificationPermissionDelegate

- (void)_webView:(WKWebView *)webView requestNotificationPermissionForSecurityOrigin:(WKSecurityOrigin *)securityOrigin decisionHandler:(void (^)(BOOL))decisionHandler
{
    decisionHandler(true);
}

- (void)webView:(WKWebView *)webView runJavaScriptAlertPanelWithMessage:(NSString *)message initiatedByFrame:(WKFrameInfo *)frame completionHandler:(void (^)(void))completionHandler
{
    alertReceived = true;
    completionHandler();
}

@end

namespace TestWebKitAPI {

static RetainPtr<NSURL> testWebPushDaemonLocation()
{
    return [currentExecutableDirectory() URLByAppendingPathComponent:@"webpushd" isDirectory:NO];
}

#if HAVE(OS_LAUNCHD_JOB)

static RetainPtr<xpc_object_t> testWebPushDaemonPList(NSURL *storageLocation)
{
    auto currentDirectory = currentExecutableDirectory();

    auto plist = adoptNS(xpc_dictionary_create(nullptr, nullptr, 0));
    xpc_dictionary_set_string(plist.get(), "_ManagedBy", "TestWebKitAPI");
    xpc_dictionary_set_string(plist.get(), "Label", "org.webkit.webpushtestdaemon");
    xpc_dictionary_set_bool(plist.get(), "LaunchOnlyOnce", true);
    xpc_dictionary_set_bool(plist.get(), "RootedSimulatorPath", true);
    xpc_dictionary_set_string(plist.get(), "StandardErrorPath", [storageLocation URLByAppendingPathComponent:@"daemon_stderr"].path.fileSystemRepresentation);

    {
        auto environmentVariables = adoptNS(xpc_dictionary_create(nullptr, nullptr, 0));
        xpc_dictionary_set_string(environmentVariables.get(), "DYLD_FRAMEWORK_PATH", currentDirectory.get().fileSystemRepresentation);
        xpc_dictionary_set_value(plist.get(), "EnvironmentVariables", environmentVariables.get());
    }
    {
        auto machServices = adoptNS(xpc_dictionary_create(nullptr, nullptr, 0));
        xpc_dictionary_set_bool(machServices.get(), "org.webkit.webpushtestdaemon.service", true);
        xpc_dictionary_set_value(plist.get(), "MachServices", machServices.get());
    }
    {
        auto programArguments = adoptNS(xpc_array_create(nullptr, 0));
        auto executableLocation = testWebPushDaemonLocation();
#if PLATFORM(MAC)
        xpc_array_set_string(programArguments.get(), XPC_ARRAY_APPEND, executableLocation.get().fileSystemRepresentation);
#else
        xpc_array_set_string(programArguments.get(), XPC_ARRAY_APPEND, [currentDirectory URLByAppendingPathComponent:@"webpushd"].path.fileSystemRepresentation);
#endif
        xpc_array_set_string(programArguments.get(), XPC_ARRAY_APPEND, "--machServiceName");
        xpc_array_set_string(programArguments.get(), XPC_ARRAY_APPEND, "org.webkit.webpushtestdaemon.service");
        xpc_dictionary_set_value(plist.get(), "ProgramArguments", programArguments.get());
    }
    return plist;
}

#else // HAVE(OS_LAUNCHD_JOB)

static RetainPtr<NSDictionary> testWebPushDaemonPList(NSURL *storageLocation)
{
    return @{
        @"Label" : @"org.webkit.pcmtestdaemon",
        @"LaunchOnlyOnce" : @YES,
        @"StandardErrorPath" : [storageLocation URLByAppendingPathComponent:@"daemon_stderr"].path,
        @"EnvironmentVariables" : @{ @"DYLD_FRAMEWORK_PATH" : currentExecutableDirectory().get().path },
        @"MachServices" : @{ @"org.webkit.webpushtestdaemon.service" : @YES },
        @"ProgramArguments" : @[
            testWebPushDaemonLocation().get().path,
            @"--machServiceName",
            @"org.webkit.webpushtestdaemon.service"
        ]
    };
}

#endif // HAVE(OS_LAUNCHD_JOB)

static NSURL *setUpTestWebPushD()
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSURL *tempDir = [NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:@"WebPushDaemonTest"] isDirectory:YES];
    NSError *error = nil;
    if ([fileManager fileExistsAtPath:tempDir.path])
        [fileManager removeItemAtURL:tempDir error:&error];
    EXPECT_NULL(error);

    killFirstInstanceOfDaemon(@"webpushd");

    auto plist = testWebPushDaemonPList(tempDir);
#if HAVE(OS_LAUNCHD_JOB)
    registerPlistWithLaunchD(WTFMove(plist));
#else
    registerPlistWithLaunchD(WTFMove(plist), tempDir);
#endif

    return tempDir;
}

static void cleanUpTestWebPushD(NSURL *tempDir)
{
    killFirstInstanceOfDaemon(@"webpushd");

    if (![[NSFileManager defaultManager] fileExistsAtPath:tempDir.path])
        return;

    NSError *error = nil;
    [[NSFileManager defaultManager] removeItemAtURL:tempDir error:&error];

    if (error)
        NSLog(@"Error removing tempDir URL: %@", error);

    EXPECT_NULL(error);
}

// FIXME: Re-enable this test for Monterey+ once webkit.org/232857 is resolved.
#if __MAC_OS_X_VERSION_MIN_REQUIRED < 110000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 120000
TEST(WebPushD, DISABLED_BasicCommunication)
#else
TEST(WebPushD, BasicCommunication)
#endif
{
    NSURL *tempDir = setUpTestWebPushD();

    auto connection = adoptNS(xpc_connection_create_mach_service("org.webkit.webpushtestdaemon.service", dispatch_get_main_queue(), 0));

    __block bool done = false;
    xpc_connection_set_event_handler(connection.get(), ^(xpc_object_t request) {
        if (xpc_get_type(request) != XPC_TYPE_DICTIONARY)
            return;
        const char* debugMessage = xpc_dictionary_get_string(request, "debug message");
        if (!debugMessage)
            return;

        bool stringMatches = !strcmp(debugMessage, "[webpushd - TestWebKitAPI] Turned Debug Mode on");
        if (!stringMatches)
            stringMatches = !strcmp(debugMessage, "[webpushd - com.apple.WebKit.TestWebKitAPI] Turned Debug Mode on");

        EXPECT_TRUE(stringMatches);

        done = true;
    });

    xpc_connection_activate(connection.get());

    audit_token_t token = { 0, 0, 0, 0, 0, 0, 0, 0 };
    mach_msg_type_number_t auditTokenCount = TASK_AUDIT_TOKEN_COUNT;
    kern_return_t result = task_info(mach_task_self(), TASK_AUDIT_TOKEN, (task_info_t)(&token), &auditTokenCount);
    if (result != KERN_SUCCESS) {
        EXPECT_TRUE(false);
        return;
    }

    // Send audit token
    {
        std::array<uint8_t, 40> encodedMessage;
        encodedMessage.fill(0);
        encodedMessage[0] = 32;
        memcpy(&encodedMessage[8], &token, sizeof(token));
        auto dictionary = adoptNS(xpc_dictionary_create(nullptr, nullptr, 0));
        xpc_dictionary_set_uint64(dictionary.get(), "protocol version", 1);
        xpc_dictionary_set_uint64(dictionary.get(), "message type", 5);
        xpc_dictionary_set_data(dictionary.get(), "encoded message", encodedMessage.data(), encodedMessage.size());
        xpc_connection_send_message(connection.get(), dictionary.get());
    }

    // Enable debug messages, and wait for the resulting debug message
    {
        auto dictionary = adoptNS(xpc_dictionary_create(nullptr, nullptr, 0));
        std::array<uint8_t, 1> encodedMessage { 1 };
        xpc_dictionary_set_uint64(dictionary.get(), "protocol version", 1);
        xpc_dictionary_set_uint64(dictionary.get(), "message type", 6);
        xpc_dictionary_set_data(dictionary.get(), "encoded message", encodedMessage.data(), encodedMessage.size());

        xpc_connection_send_message(connection.get(), dictionary.get());
        TestWebKitAPI::Util::run(&done);
    }

    // Echo and wait for a reply
    auto dictionary = adoptNS(xpc_dictionary_create(nullptr, nullptr, 0));
    std::array<uint8_t, 10> encodedString { 5, 0, 0, 0, 1, 'h', 'e', 'l', 'l', 'o' };
    xpc_dictionary_set_uint64(dictionary.get(), "protocol version", 1);
    xpc_dictionary_set_uint64(dictionary.get(), "message type", 1);
    xpc_dictionary_set_data(dictionary.get(), "encoded message", encodedString.data(), encodedString.size());

    done = false;
    xpc_connection_send_message_with_reply(connection.get(), dictionary.get(), dispatch_get_main_queue(), ^(xpc_object_t reply) {
        if (xpc_get_type(reply) != XPC_TYPE_DICTIONARY) {
            NSLog(@"Unexpected non-dictionary: %@", reply);
            done = true;
            EXPECT_TRUE(FALSE);
            return;
        }

        size_t dataSize = 0;
        const void* data = xpc_dictionary_get_data(reply, "encoded message", &dataSize);
        EXPECT_EQ(dataSize, 15u);
        std::array<uint8_t, 15> expectedReply { 10, 0, 0, 0, 1, 'h', 'e', 'l', 'l', 'o' , 'h', 'e', 'l', 'l', 'o' };
        EXPECT_FALSE(memcmp(data, expectedReply.data(), expectedReply.size()));
        done = true;
    });
    TestWebKitAPI::Util::run(&done);

    cleanUpTestWebPushD(tempDir);
}

static const char* mainBytes = R"WEBPUSHRESOURCE(
<script>
    Notification.requestPermission().then(() => { alert("done") })
</script>
)WEBPUSHRESOURCE";

// FIXME: Re-enable this test for Monterey+ once webkit.org/232857 is resolved.
#if __MAC_OS_X_VERSION_MIN_REQUIRED < 110000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 120000
TEST(WebPushD, DISABLED_PermissionManagement)
#else
TEST(WebPushD, PermissionManagement)
#endif
{
    NSURL *tempDirectory = setUpTestWebPushD();

    auto dataStoreConfiguration = adoptNS([_WKWebsiteDataStoreConfiguration new]);
    dataStoreConfiguration.get().webPushMachServiceName = @"org.webkit.webpushtestdaemon.service";
    auto dataStore = adoptNS([[WKWebsiteDataStore alloc] _initWithConfiguration:dataStoreConfiguration.get()]);

    auto configuration = adoptNS([[WKWebViewConfiguration alloc] init]);
    configuration.get().websiteDataStore = dataStore.get();
    [configuration.get().preferences _setNotificationsEnabled:YES];
    for (_WKExperimentalFeature *feature in [WKPreferences _experimentalFeatures]) {
        if ([feature.key isEqualToString:@"BuiltInNotificationsEnabled"])
            [[configuration preferences] _setEnabled:YES forFeature:feature];
    }

    auto handler = adoptNS([[TestURLSchemeHandler alloc] init]);
    [configuration setURLSchemeHandler:handler.get() forURLScheme:@"testing"];

    [handler setStartURLSchemeTaskHandler:^(WKWebView *, id<WKURLSchemeTask> task) {
        auto response = adoptNS([[NSURLResponse alloc] initWithURL:task.request.URL MIMEType:@"text/html" expectedContentLength:0 textEncodingName:nil]);
        [task didReceiveResponse:response.get()];
        [task didReceiveData:[NSData dataWithBytes:mainBytes length:strlen(mainBytes)]];
        [task didFinish];
    }];

    auto webView = adoptNS([[TestWKWebView alloc] initWithFrame:CGRectMake(0, 0, 800, 600) configuration:configuration.get()]);
    auto uiDelegate = adoptNS([[NotificationPermissionDelegate alloc] init]);
    [webView setUIDelegate:uiDelegate.get()];

    [webView loadRequest:[NSURLRequest requestWithURL:[NSURL URLWithString:@"testing://main/index.html"]]];
    TestWebKitAPI::Util::run(&alertReceived);

    static bool originOperationDone = false;
    static RetainPtr<WKSecurityOrigin> origin;
    [dataStore _getOriginsWithPushAndNotificationPermissions:^(NSSet<WKSecurityOrigin *> *origins) {
        EXPECT_EQ([origins count], 1u);
        origin = [origins anyObject];
        originOperationDone = true;
    }];

    TestWebKitAPI::Util::run(&originOperationDone);

    EXPECT_TRUE([origin.get().protocol isEqualToString:@"testing"]);
    EXPECT_TRUE([origin.get().host isEqualToString:@"main"]);

    // If we failed to retrieve an expected origin, we will have failed the above checks
    if (!origin) {
        cleanUpTestWebPushD(tempDirectory);
        return;
    }

    originOperationDone = false;
    [dataStore _deletePushAndNotificationRegistration:origin.get() completionHandler:^(NSError *error) {
        EXPECT_FALSE(!!error);
        originOperationDone = true;
    }];

    TestWebKitAPI::Util::run(&originOperationDone);

    originOperationDone = false;
    [dataStore _getOriginsWithPushAndNotificationPermissions:^(NSSet<WKSecurityOrigin *> *origins) {
        EXPECT_EQ([origins count], 0u);
        originOperationDone = true;
    }];
    TestWebKitAPI::Util::run(&originOperationDone);

    cleanUpTestWebPushD(tempDirectory);
}

} // namespace TestWebKitAPI

#endif // PLATFORM(MAC) || PLATFORM(IOS)
