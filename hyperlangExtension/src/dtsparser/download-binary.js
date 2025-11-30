// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

const fs = require('fs');
const path = require('path');
const https = require('https');
const { execSync } = require('child_process');

const RELEASE_TAG = 'v1.0.0';
const BINARY_URLS = {
    linux: 'https://gitcode.com/anancds123/cjbind/releases/download/v0.2.7/cjbind-linux-x64',
    win32: 'https://gitcode.com/anancds123/cjbind/releases/download/v0.2.7/cjbind-windows-x64.exe',
    darwin: 'https://gitcode.com/anancds123/cjbind/releases/download/v0.2.7/cjbind-darwin-arm64'
};


const BIN_DIR = path.resolve(__dirname, './node_modules/.bin');
const BINARY_NAME = process.platform === 'win32' ? 'cjbind.exe' : 'cjbind';
const DOWNLOAD_FILE = path.resolve(BIN_DIR, BINARY_NAME);

if (!fs.existsSync(BIN_DIR)) fs.mkdirSync(BIN_DIR, { recursive: true });

const platform = process.platform;
const binaryUrl = BINARY_URLS[platform];
if (!binaryUrl) {
    console.error(`unsupported system: ${platform}`);
    process.exit(1);
}

console.log(`download ${RELEASE_TAG} binary (${platform}): ${binaryUrl} to ${DOWNLOAD_FILE}`);

const writeStream = fs.createWriteStream(DOWNLOAD_FILE);
let redirectCount = 0;
function fetchUrl(currentUrl, options = {}) {

    const { token, timeout = 300000, maxRedirects = 5 } = options;

    const requestOptions = {
        timeout,
        headers: {}
    };


    if (token) {
        requestOptions.headers.Authorization = `token ${token}`;
    }

    https.get(currentUrl, requestOptions, (res) => {
        switch (res.statusCode) {
            case 301:
            case 302: {
                redirectCount++;
                if (redirectCount > maxRedirects) {
                    writeStream.destroy();
                    fs.existsSync(DOWNLOAD_FILE) && fs.unlinkSync(DOWNLOAD_FILE);
                    console.error(`download failed, after (${maxRedirects}) times.`);
                    return;
                }

                // 从响应头获取新的重定向 URL
                const redirectUrl = res.headers.location;
                if (!redirectUrl) {
                    writeStream.destroy();
                    fs.existsSync(DOWNLOAD_FILE) && fs.unlinkSync(DOWNLOAD_FILE);
                    console.error('download failed:302, response has no redirect location.');
                    return;
                }

                console.log(`redirect (${res.statusCode}), to ${redirectUrl}`);
                fetchUrl(redirectUrl);
                break;
            }

            case 200: {
                console.log(`begin downd: ${currentUrl}`);
                const totalSize = parseInt(res.headers['content-length'], 10) || 0;
                let downloadedSize = 0;
                console.log(`file size:${totalSize ? (totalSize / 1024 / 1024).toFixed(2) + ' MB' : 'unknown'}`);

                res.on('data', (chunk) => {
                    downloadedSize += chunk.length;
                    if (totalSize) {
                        const progress = (downloadedSize / totalSize) * 100;
                        process.stdout.write(`\rdownload progress:${progress.toFixed(2)}%(${(downloadedSize / 1024 / 1024).toFixed(2)} MB)`);
                    }
                });

                res.pipe(writeStream);

                writeStream.on('finish', () => {
                    process.stdout.write('\n');
                    fs.chmodSync(DOWNLOAD_FILE, 0o755);
                    console.log(`write success: ${DOWNLOAD_FILE}`);
                });
                break;
            }

            default: {
                writeStream.destroy();
                fs.existsSync(DOWNLOAD_FILE) && fs.unlinkSync(DOWNLOAD_FILE);
                console.error(`download failed, status code: ${res.statusCode}, URL: ${currentUrl}`);
                break;
            }
        }
    })
        .on('error', (err) => {
            writeStream.destroy();
            fs.existsSync(DOWNLOAD_FILE) && fs.unlinkSync(DOWNLOAD_FILE);
            console.error(`download error:${err.message}, URL: ${currentUrl}`);
        })
        .on('timeout', () => {
            writeStream.destroy();
            fs.existsSync(DOWNLOAD_FILE) && fs.unlinkSync(DOWNLOAD_FILE);
            console.error(`download timeout (${timeout}ms), URL:${currentUrl}`);
        });
}

writeStream.on('error', (err) => {
    console.error(`file write failed: ${err.message}`);
    fs.existsSync(DOWNLOAD_FILE) && fs.unlinkSync(DOWNLOAD_FILE);
});

fetchUrl(binaryUrl)