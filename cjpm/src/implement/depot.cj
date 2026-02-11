// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.net.*
import std.io.*
import std.fs.*
import std.collection.*
import stdx.net.http.*
import stdx.net.tls.common.*
import stdx.net.tls.*
import stdx.compress.*
import stdx.crypto.digest.*
import stdx.encoding.hex.*
import stdx.serialization.serialization.*
import stdx.encoding.json.*
import cjpm.config.*
import stdx.crypto.common.CryptoException

const CONNECT_TIMEOUT = 30
const READ_TIMEOUT = 60

let tcpSocketConnector = {
    sa: SocketAddress =>
        let socket = TcpSocket(sa)
        socket.connect(timeout: Duration.second * CONNECT_TIMEOUT)
        return socket
}

public class Depot {
    public var baseUrl: String = ""
    public var token: String = ""
    public var localRepo: String = ""

    public func loadConfiguration(isError!: Bool = true): Bool {
        let alertLevel =  if (isError) { "Error" } else { "Warning" }
        if (!FINAL_CJPM_CONFIG_INFO.repository.contains(REPO_HOME_NAME)) {
            eprintln("${alertLevel}: get data of [repository.home] failed, please check if cangjie-repo.toml is set up correctly")
            return false
        }

        this.baseUrl = FINAL_CJPM_CONFIG_INFO.repository[REPO_HOME_NAME].registry
        this.token = FINAL_CJPM_CONFIG_INFO.repository[REPO_HOME_NAME].token
        this.localRepo = Path(FINAL_CJPM_CONFIG_INFO.repository[REPO_CACHE_NAME].path).join("repository").toString()

        if (baseUrl.isEmpty()) {
            eprintln("${alertLevel}: get registry of [repository.home] failed, please check if cangjie-repo.toml is set up correctly")
            return false
        }

        return true
    }

    public static func new(): Option<Depot> {
        let instance: Depot = Depot()
        if (instance.loadConfiguration()) {
            return Option<Depot>.Some(instance)
        }
        return Option<Depot>.None
    }

    // publish data to central repository
    public func publish(tomlInfo: TomlInfo, data: Array<Byte>): (Bool, String) {
        let name = tomlInfo.pkg.name
        let version = tomlInfo.pkg.version
        let org = tomlInfo.pkg.organization

        // check name and organization
        var flag = if (org.isEmpty()) { true } else { checkBundleName(org, "organization") }
        flag = checkBundleName(name, "module") && flag
        if (!flag) {
            return (false, "")
        }

        var publishUrl = baseUrl + "/pkg/${name}"
        var orgStr = ""
        if (!org.isEmpty()) {
            publishUrl += "?organization=${org}"
            orgStr = "to organization '${org}' "
        }

        // building client
        var tlsConfig = TlsClientConfig()
        tlsConfig.verifyMode = TrustAll
        let client = ClientBuilder().
            connector(tcpSocketConnector).
            noProxy().
            readTimeout(Duration.second * READ_TIMEOUT).
            tlsConfig(tlsConfig).
            build()

        // building https request
        var body = ByteBuffer()
        body.write(data)
        let request = HttpRequestBuilder().
            method("POST").
            url(publishUrl).
            header("Authorization", token).
            body(body).
            build()
        let resp = client.send(request)
        client.close()

        // verify response code
        match (resp.status) {
            case 200 => return (true, "${name}-${version} ${orgStr}publish success")
            case 400 => return (false, "${name}-${version} ${orgStr}publish failed with status ${resp.status}: invalid parameters")
            case 401 => return (false, "${name}-${version} ${orgStr}publish failed with status ${resp.status}: authentication failed")
            case 403 => return (false, "${name}-${version} ${orgStr}publish failed with status ${resp.status}: permission denied")
            case 404 => return (false, "${name}-${version} ${orgStr}publish failed with status ${resp.status}: file does not exist")
            case 409 => return (false, "${name}-${version} ${orgStr}publish failed with status ${resp.status}: artifact with the same name and verison already exists")
            case s where (s >= 500) => return (false, "${name}-${version} ${orgStr}publish failed with status ${resp.status}: server error")
            case _ => return (false, "${name}-${version} ${orgStr}publish failed, unknown error with status ${resp.status}")
        }
    }

    // download package from central repository
    public func download(name: String, org: String, version: String): (Bool, String) {
        var downloadUrl = baseUrl + "/pkg/${name}/${version}"
        var orgStr = ""
        if (!org.isEmpty()) {
            downloadUrl += "?organization=${org}"
            orgStr = "from organization '${org}' "
        }

        // building client
        var tlsConfig = TlsClientConfig()
        tlsConfig.verifyMode = TrustAll
        let client = ClientBuilder().
            connector(tcpSocketConnector).
            noProxy().
            readTimeout(Duration.second * READ_TIMEOUT).
            tlsConfig(tlsConfig).
            build()

        // building https request
        let request = HttpRequestBuilder().method("GET").url(downloadUrl).build()
        let resp = client.send(request)

        // verify response code
        var (flag, errLog) = (true, "")
        match (resp.status) {
            case 200 => ()
            case 400 => (flag, errLog) = (false, "${name}-${version} ${orgStr}load failed with status ${resp.status}: invalid parameters")
            case 401 => (flag, errLog) = (false, "${name}-${version} ${orgStr}load failed with status ${resp.status}: authentication failed")
            case 403 => (flag, errLog) = (false, "${name}-${version} ${orgStr}load failed with status ${resp.status}: permission denied")
            case 404 => (flag, errLog) = (false, "${name}-${version} ${orgStr}load failed with status ${resp.status}: file does not exist")
            case s where (s >= 500) => (flag, errLog) = (false, "${name}-${version} ${orgStr}load failed with status ${resp.status}: server error")
            case _ => (flag, errLog) = (false, "${name}-${version} ${orgStr}load failed, unknown error with status ${resp.status}")
        }
        if (!flag) {
            client.close()
            return (flag, errLog)
        }

        let buf = Array<UInt8>(1024, repeat: 0)
        let buffer = ArrayList<UInt8>()
        var i = 0
        do {
            i = resp.body.read(buf)
            buffer.add(all: buf[..i])
        } while (i != 0)
        client.close()

        // check sha256sum
        try {
            let sha256Instance = SHA256()
            sha256Instance.write(buffer.toArray())
            let sha256sum = toHexString(sha256Instance.finish())
            let indexPath = this.getIndex(name, org)
            if (indexPath.isEmpty()) {
                return (false, "index error while downloading ${name}-${version} ${orgStr}")
            }

            var indexRawData = String.fromUtf8(File.readFrom(indexPath))
            var indexIter = indexRawData.lines()

            var checkSumFlag = false
            while (true) {
                match (indexIter.next()) {
                    case Some(indexStr) =>
                        try {
                            let index: ArtifactIndex = ArtifactIndex.deserialize(DataModel.fromJson(JsonValue.fromStr(indexStr)))
                            if (index.version != version) {
                                continue
                            }
                            if (index.sha256sum != sha256sum) {
                                return (false, "package was broken while downloading ${name}-${version} ${orgStr}, please try 'cjpm update'")
                            }
                            checkSumFlag = true
                            break
                        } catch (e: Exception) {
                            return (false, "index was broken while downloading ${name}-${version} ${orgStr}, please try 'cjpm update'")
                        }
                    case None => break
                }
            }

            if (!checkSumFlag) {
                return (false, "cannot find index data while downloading ${name}-${version} ${orgStr}, please try 'cjpm update'")
            }
        } catch (e: CryptoException) {
            eprintln("Error: ${e.message}")
            eprintln("Error: please ensure that openssl has been correctly installed and configured according to the \"Usage Instruction\" section of the documentation")
            return (false, "failed to check sha256 check-sum due to missing OpenSSL")
        } catch (e: Exception) {
            return (false, "failed to check sha256 check-sum while downloading ${name}-${version} ${orgStr}, please try 'cjpm update'")
        }

        // save, uncompress and delete cjp file
        let repositoryPath = Path(this.localRepo).join("source").
            join(if (org.isEmpty()) { "default" } else { org }).
            toString()
        let cjpTempPath = Path(repositoryPath).join("${name}-${version}.cjp")
        try {
            // save
            if (!createDirectory(repositoryPath)) {
                return (false, "${name}-${version} load failed with error while uncompressing and saving repository cache")
            }
            File.writeTo(cjpTempPath, buffer.toArray())

            // uncompress
            TarGzip.extract(fromTarGzip: cjpTempPath.toString(), destDir: repositoryPath, overwrite: true)
            return (true, "${name}-${version} load success")
        } catch (e: Exception) {
            eprintln(e.message)
            return (false, "${name}-${version} load failed with error while uncompressing and saving repository cache")
        } finally {
            // delete cjp
            deleteFile(cjpTempPath.toString())
        }
    }

    // update local index for one module with output
    public func updateIndexWithOutput(name: String, org: String): (Bool, String) {
        let firstLevel = name[..2]
        let secondLevel = if (name.size > 3) { name[2..4] } else { name[2..3] }
        var indexUrl = baseUrl + "/index/${firstLevel}/${secondLevel}/${name}"
        var orgStr = ""
        if (!org.isEmpty()) {
            indexUrl += "?organization=${org}"
            orgStr = " from organization '${org}'"
        }

        // building client
        var tlsConfig = TlsClientConfig()
        tlsConfig.verifyMode = TrustAll
        let client = ClientBuilder().
            connector(tcpSocketConnector).
            noProxy().
            readTimeout(Duration.second * READ_TIMEOUT).
            tlsConfig(tlsConfig).
            build()

        // building https request
        let request = HttpRequestBuilder().method("GET").url(indexUrl).build()
        let resp = client.send(request)

        // verify response code
        var (flag, errLog) = (true, "")
        match (resp.status) {
            case 200 => ()
            case 400 =>
                (flag, errLog) = (false, "index of module '${name}'${orgStr} load failed with status ${resp.status}: invalid parameters")
            case 401 =>
                (flag, errLog) = (false, "index of module '${name}'${orgStr} load failed with status ${resp.status}: authentication failed")
            case 403 =>
                (flag, errLog) = (false, "index of module '${name}'${orgStr} load failed with status ${resp.status}: permission denied")
            case 404 =>
                (flag, errLog) = (false, "index of module '${name}'${orgStr} load failed with status ${resp.status}: file does not exist")
            case s where (s >= 500) =>
                (flag, errLog) = (false, "index of module '${name}'${orgStr} load failed with status ${resp.status}: server error")
            case _ =>
                (flag, errLog) = (false, "index of module '${name}'${orgStr} load failed, unknown error with status ${resp.status}")
        }
        if (!flag) {
            client.close()
            return (flag, errLog)
        }

        let buf = Array<UInt8>(1024, repeat: 0)
        let buffer = ArrayList<UInt8>()
        var i = 0
        do {
            i = resp.body.read(buf)
            buffer.add(all: buf[..i])
        } while (i != 0)
        client.close()

        // save index file
        let indexDir = Path(this.localRepo).join("index").
            join(if (org.isEmpty()) { "default" } else { org }).
            join(firstLevel).
            join(secondLevel).
            toString()
        let indexPath = Path(indexDir).join(name)
        if (!createDirectory(indexDir)) {
            return (false, "cannot save index file for module '${name}'${orgStr}")
        }
        File.writeTo(indexPath, buffer.toArray())

        return (true, "")
    }

    // update local index for one module
    public func updateIndex(name: String, org: String): Bool {
        let (flag, output) = this.updateIndexWithOutput(name, org)
        if (!flag) {
            eprintln("Error: ${output}")
        }
        return flag
    }

    // get index file
    public func getIndex(name: String, org: String): String {
        let firstLevel = name[..2]
        let secondLevel = if (name.size > 3) { name[2..4] } else { name[2..3] }
        let indexPath = Path(this.localRepo).join("index").
            join(if (org.isEmpty()) { "default" } else { org }).
            join(firstLevel).
            join(secondLevel).
            join(name).toString()
        if (fileExists(indexPath)) {
            return indexPath
        }
        return ""
    }
}
