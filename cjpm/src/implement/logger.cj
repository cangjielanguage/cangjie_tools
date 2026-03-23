// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.concurrent.*
import std.fs.*

import cjpm.config.*

private enum MessageKind {
    | Release
    | Print(String, String)
    | Console(String)
}

public class BoundedFileLogger {
    private let messageQueue = LinkedBlockingQueue<MessageKind>()
    private var logFuture: ?Future<Bool> = None

    public BoundedFileLogger(
        private var logBound: Int64
    ) { }

    public func release(): Unit {
        this.messageQueue.add(Release)
    }

    public func log(logFiles: ?(String, String)): Unit {
        if (let Some((outLogFile, errLogFile)) <- logFiles) {
            this.messageQueue.add(Print(outLogFile, errLogFile))
        }
    }

    public func console(msg: String): Unit {
        this.messageQueue.add(Console(msg))
    }

    public func start(): Unit {
        if (logFuture.isNone()) {
            this.logFuture = spawn { =>
                var count = this.logBound
                var success: Bool = true
                while (count > 0) {
                    match (this.messageQueue.remove()) {
                        case Release => count--
                        case Print(outLogFile, errLogFile) =>
                            success = printLogs(outLogFile, errLogFile) && success
                        case Console(msg) => println(msg)
                    }
                }
                return success
            }
        }
    }

    // If number of `log` and `release` calls is below the `messageBound`, deadlock occurs
    func unsafeWait(): Bool {
        if (let Some(logFuture) <- this.logFuture) {
            logFuture.get()
        } else { true }
    }


    private static func printLogs(outLogFile: String, errLogFile: String): Bool {
        var logFlag: Bool = true
        func printLogFile(logFile: String, toStrerr: Bool): Bool {
            if (!fileExists(logFile)) {
                return true
            }
            try {
                let logStr: String = unsafe { String.fromUtf8Unchecked(File.readFrom(logFile)) }
                if (toStrerr) {
                    eprint(logStr)
                } else {
                    print(logStr)
                }
            } catch (e: Exception) {
                eprintln(e.message)
                eprintln("Error: failed to read '${logFile}'")
                return false
            }
            return true
        }
        logFlag = printLogFile(outLogFile, false) && logFlag
        logFlag = printLogFile(errLogFile, true) && logFlag
        return logFlag
    }
}

func deleteLog(logFiles: ?(String, String)): Bool {
    var delLog: Bool = true
    if (let Some((outLogFile, errLogFile)) <- logFiles) {
        if (!deleteFile(outLogFile)) {
            delLog = false
        }
        if (!deleteFile(errLogFile)) {
            delLog = false
        }
    }
    return delLog
}