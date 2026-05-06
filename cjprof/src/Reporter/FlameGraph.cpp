// Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0 
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "Reporter/FlameGraph.h"

const std::string FlameGraph::m_svgTemplate = R"###(<?xml version="1.0" standalone="no"?>
<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">
<svg version="1.1" width="${width}" height="${height}" onload="init(evt)" viewBox="0 0 ${width} ${height}"
    xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">
<defs>
    <linearGradient id="background" y1="0" y2="1" x1="0" x2="0" >
        <stop stop-color="#eeeeee" offset="5%" />
        <stop stop-color="#eeeeb0" offset="95%" />
    </linearGradient>
</defs>
<style type="text/css">
    text { font-family:Verdana; font-size:${fontsize}px; fill:rgb(0, 0, 0); }
    #search, #ignorecase { opacity:0.1; cursor:pointer; }
    #search:hover, #search.show, #ignorecase:hover, #ignorecase.show { opacity:1; }
    #title { text-anchor:middle; font-size:${titlesize}px}
    #unzoom { cursor:pointer; }
    #frames > *:hover { stroke:black; stroke-width:0.5; cursor:pointer; }
    .hide { display:none; }
    .parent { opacity:0.5; }
</style>
<script type="text/ecmascript">
<![CDATA[
    "use strict";
    var details, searchbtn, unzoombtn, matchedtxt, svg, searching, currentSearchTerm, ignorecase, ignorecaseBtn;
    function init(evt) {
        details = document.getElementById("details").firstChild;
        ignorecaseBtn = document.getElementById("ignorecase");
        searchbtn = document.getElementById("search");
        matchedtxt = document.getElementById("matched");
        unzoombtn = document.getElementById("unzoom");
        svg = document.getElementsByTagName("svg")[0];

        // init flamegraph state
        searching = 0;
        currentSearchTerm = null;

        // restore flamegraph state
        var keyValues = get_params();
        if (keyValues.x && keyValues.y) {
            zoom(find_group(document.querySelector('[x="' + keyValues.x + '"][y="' + keyValues.y + '"]')));
        }
        if (keyValues.s) {
            search(keyValues.s);
        }
    }

    // add click event listener
    window.addEventListener("click", function(e) {
        var target = find_group(e.target);
        if (target) {
            if (target.nodeName == "a") {
                if (e.ctrlKey === false) {
                    return;
                }
                e.preventDefault();
            }
            if (target.classList.contains("parent")) {
                unzoom(true);
            }
            zoom(target);
            if (!document.querySelector(".parent")) {
                var keyValues = get_params();
                if (keyValues.x) {
                    delete keyValues.x;
                }
                if (keyValues.y) {
                    delete keyValues.y;
                }
                history.replaceState(null, null, parse_params(keyValues));
                unzoombtn.classList.add("hide");
                return;
            }

            var rect_ele = target.querySelector("rect");
            if (rect_ele && rect_ele.attributes && rect_ele.attributes.y && rect_ele.attributes._orig_x) {
                var keyValues = get_params();
                keyValues.y = rect_ele.attributes.y.value;
                keyValues.x = rect_ele.attributes._orig_x.value;
                history.replaceState(null, null, parse_params(keyValues));
            }
        }
        else if (e.target.id == "ignorecase") {
            toggle_ignorecase();
        }
        else if (e.target.id == "unzoom") {
            clearzoom();
        }
        else if (e.target.id == "search") {
            search_prompt();
        }
    }, false)

    // add mouseover event listener
    window.addEventListener("mouseover", function(e) {
        var target = find_group(e.target);
        if (target) {
            details.nodeValue = "Function: " + g_to_text(target);
        }
    }, false)

    // add mouseout event listener
    window.addEventListener("mouseout", function(e) {
        var target = find_group(e.target);
        if (target) {
            details.nodeValue = ' ';
        }
    }, false)

    // add keydown event listener
    window.addEventListener("keydown", function(e) {
        // open search prompt: Ctrl + F
        if (e.keyCode === 114 || (e.ctrlKey && e.keyCode === 70)) {
            e.preventDefault();
            search_prompt();
        }
        // toggle ignore case for search: Ctrl + I
        else if (e.ctrlKey && e.keyCode === 73) {
            e.preventDefault();
            toggle_ignorecase();
        }
    }, false)

    // some functions
    // get keyValues from url
    function get_params() {
        var keyValues = {};
        var url_search = window.location.search;
        var items_array = url_search.substr(1).split('&');
        for (var i = 0; i < items_array.length; ++i) {
            var keyValue = items_array[i].split("=");
            if (!keyValue[0] || !keyValue[1]) {
                continue;
            }
            keyValues[keyValue[0]] = decodeURIComponent(keyValue[1]);
        }
        return keyValues;
    }

    // build url forme keyValues
    function parse_params(keyValues) {
        var url_search = "?";
        for (var key in keyValues) {
            var keyValue = key + '=' + encodeURIComponent(keyValues[key]) + '&';
            url_search += keyValue;
        }
        if (url_search.slice(-1) == "&") {
            url_search = url_search.substring(0, url_search.length - 1);
        }
        if (url_search == '?') {
            url_search = window.location.href.split('?')[0];
        }
        return url_search;
    }

    // find first child
    function find_child(node, selector) {
        var children = node.querySelectorAll(selector);
        if (children.length > 0) {
            return children[0];
        }
    }

    function find_group(node) {
        var parent = node.parentElement;
        if (!parent) {
            return;
        }
        if (parent.id == "frames") {
            return node;
        }
        return find_group(parent);
    }
    
    // save attr value
    function orig_attr_save(e, attr, val) {
        if (e.attributes[attr] == undefined) {
            return;
        }
        var orig_attr_key = "_orig_" + attr;
        if (e.attributes[orig_attr_key] != undefined) {
            return;
        }
        if (val == undefined) {
            val = e.attributes[attr].value;
        }
        e.setAttribute(orig_attr_key, val);
    }

    // load attr orig value
    function orig_attr_load(e, attr) {
        var orig_attr_key = "_orig_" + attr;
        if (e.attributes[orig_attr_key] == undefined) {
            return;
        }
        e.attributes[attr].value = e.attributes[orig_attr_key].value;
        e.removeAttribute(orig_attr_key);
    }

    function g_to_text(e) {
        var text = find_child(e, "title").firstChild.nodeValue;
        return (text);
    }
    
    function g_to_func(e) {
        var func = g_to_text(e);
        // process func
        return (func);
    }
    
    function update_text(e) {
        var child_rect = find_child(e, "rect");
        var child_text = find_child(e, "text");

        // get available rect width
        var w = parseFloat(child_rect.attributes.width.value) - 3;
        
        // get show txt
        var txt = find_child(e, "title").textContent.replace(/\([^(]*\)$/, "");

        child_text.attributes.x.value = parseFloat(child_rect.attributes.x.value) + 3;

        // if w is too small, hide text
        if (w < 2 * ${fontwidth} * ${fontsize}) {
            child_text.textContent = "";
            return;
        }

        child_text.textContent = txt;
        
        // if we can show entire txt with w or txt only has spaces
        if (/^ *$/.test(txt)) {
            return;
        }

        var sl = child_text.getSubStringLength(0, txt.length);
        if (w > sl) {
            return;
        }

        // compute show ratiom, show the longest txt
        var start = Math.floor((w / sl) * txt.length);
        for (var x = start; x > 0; x = x - 2) {
            if (child_text.getSubStringLength(0, x + 2) <= w) {
                child_text.textContent = txt.substring(0, x) + "..";
                return;
            }
        }
        child_text.textContent = "";
    }
    
    // zoom
    function zoom_reset(e) {
        if (e.attributes != undefined) {
            orig_attr_load(e, "x");
            orig_attr_load(e, "width");
        }
        if (e.childNodes == undefined) {
            return;
        }
        for (var i = 0, c = e.childNodes; i < c.length; ++i) {
            zoom_reset(c[i]);
        }
    }

    function zoom_child(e, x, ratio) {
        if (e.attributes != undefined) {
            if (e.attributes.width != undefined) {
                orig_attr_save(e, "width");
                e.attributes.width.value = parseFloat(e.attributes.width.value) * ratio;
            }
            if (e.attributes.x != undefined) {
                orig_attr_save(e, "x");
                e.attributes.x.value = (parseFloat(e.attributes.x.value) - x - ${xpad}) * ratio + ${xpad};
                if (e.tagName == "text") {
                    e.attributes.x.value = find_child(e.parentNode, "rect[x]").attributes.x.value + 3;
                }
            }
        }

        if (e.childNodes == undefined) {
            return;
        }
        for (var i = 0, c = e.childNodes; i < c.length; ++i) {
            zoom_child(c[i], x - ${xpad}, ratio);
        }
    }

    function zoom_parent(e) {
        if (e.attributes) {
            if (e.attributes.x != undefined) {
                orig_attr_save(e, "x");
                e.attributes.x.value = ${xpad};
            }
            if (e.attributes.width != undefined) {
                orig_attr_save(e, "width");
                e.attributes.width.value = parseInt(svg.width.baseVal.value) - (${xpad} * 2);
            }
        }
        if (e.childNodes == undefined) {
            return;
        }
        for (var i = 0, childs = e.childNodes; i < childs.length; ++i) {
            zoom_parent(childs[i]);
        }
    }

    function zoom(node) {
        var rect_attr = find_child(node, "rect").attributes;
        var width = parseFloat(rect_attr.width.value);
        var xmin = parseFloat(rect_attr.x.value);
        var xmax = parseFloat(xmin + width);
        var ymin = parseFloat(rect_attr.y.value);

        // compute zoom ratio
        var ratio = (svg.width.baseVal.value - 2 * ${xpad}) / width;

        var delta_float = 0.0001;

        unzoombtn.classList.remove("hide");

        var elements = document.getElementById("frames").children;
        for (var i = 0; i < elements.length; ++i) {
            var ele = elements[i];
            var attr = find_child(ele, "rect").attributes;
            var attr_x = parseFloat(attr.x.value);
            var attr_w = parseFloat(attr.width.value);
            var upstack = parseFloat(attr.y.value) > ymin;
            if (upstack) {
                if (attr_x <= xmin && (attr_x + attr_w + delta_float) >= xmax) {
                    ele.classList.add("parent");
                    zoom_parent(ele);
                    update_text(ele);
                }
                else {
                    ele.classList.add("hide");
                }
            }
            else {
                if (attr_x < xmin || (attr_x + delta_float) >= xmax) {
                    ele.classList.add("hide");
                }
                else {
                    zoom_child(ele, xmin, ratio);
                    update_text(ele);
                }
            }
        }
        search();
    }

    function unzoom(dont_update_text) {
        unzoombtn.classList.add("hide");
        var elements = document.getElementById("frames").children;
        for (var i = 0; i < elements.length; ++i) {
            elements[i].classList.remove("parent");
            elements[i].classList.remove("hide");
            zoom_reset(elements[i]);
            if (!dont_update_text) {
                update_text(elements[i]);
            }
        }
        search();
    }

    function clearzoom() {
        unzoom();

        var keyValues = get_params();
        if (keyValues.x) {
            delete keyValues.x;
        }
        if (keyValues.y) {
            delete keyValues.y;
        }
        history.replaceState(null, null, parse_params(keyValues));
    }

    function toggle_ignorecase() {
        ignorecase = !ignorecase;
        if (ignorecase) {
            ignorecaseBtn.classList.add("show");
        } else {
            ignorecaseBtn.classList.remove("show");
        }
        reset_search();
        search();
    }

    function reset_search() {
        var elements = document.querySelectorAll("#frames rect");
        for (var i = 0; i < elements.length; ++i) {
            orig_attr_load(elements[i], "fill")
        }
        var keyValues = get_params();
        delete keyValues.s;
        history.replaceState(null, null, parse_params(keyValues));
    }

    // show search prompt
    function search_prompt() {
        if (!searching) {
            var term = prompt("Enter a search term (regexp " +
                "allowed, eg: ^ext4_)"
                + (ignorecase ? ", ignoring case sensitivity" : "")
                + "\nPress Ctrl+I to toggle case sensitivity", "");
            if (term != null) {
                search(term);
            }
        } else {
            reset_search();
            searching = 0;
            currentSearchTerm = null;
            searchbtn.classList.remove("show");
            searchbtn.firstChild.nodeValue = "Search";
            matchedtxt.classList.add("hide");
            matchedtxt.firstChild.nodeValue = "";
        }
    }

    function search(term) {
        if (term) {
            currentSearchTerm = term;
        }

        var searchRegex = new RegExp(currentSearchTerm, ignorecase ? 'i' : '');
        var matches = new Object();
        var maxwidth = 0;
        var elements = document.getElementById("frames").children;
        for (var i = 0; i < elements.length; ++i) {
            var ele = elements[i];
            var func = g_to_func(ele);
            if (func == null) {
                continue;
            }
            var rect = find_child(ele, "rect");
            if (rect == null) {
                continue;
            }

            // record max width
            var w = parseFloat(rect.attributes.width.value);
            if (w > maxwidth) {
                maxwidth = w;
            }
            
            // regex match func name
            if (func.match(searchRegex)) {
                var x = parseFloat(rect.attributes.x.value);
                
                // save original color to reset
                orig_attr_save(rect, "fill");
                
                // highlight match rect
                rect.attributes.fill.value = "rgb(230,0,230)";
                // find max match width
                if (matches[x] == undefined) {
                    matches[x] = w;
                } else {
                    matches[x] = Math.max(matches[x], w);
                }
                searching = 1;
            }
        }
        // search fail
        if (!searching) {
            return;
        }
        
        // update url state
        var keyValues = get_params();
        keyValues.s = currentSearchTerm;
        history.replaceState(null, null, parse_params(keyValues));

        // update button state
        searchbtn.classList.add("show");
        searchbtn.firstChild.nodeValue = "Reset Search";

        // compute match ratio
        var lastx = -1;
        var lastw = 0;
        var count = 0;
        var keys = Array();

        // get all match x location
        for (key in matches) {
            if (matches.hasOwnProperty(key)) {
                keys.push(key);
            }
        }
        
        keys.sort(function(a, b) {
            return a - b;
        });
        
        var delta_float = 0.0001;
        for (var k in keys) {
            var x = parseFloat(keys[k]);
            var w = matches[keys[k]];
            if (x >= (lastx + lastw - delta_float)) {
                count += w;
                lastx = x;
                lastw = w;
            }
        }
        // show match percent
        matchedtxt.classList.remove("hide");
        var ratio = 100 * count / maxwidth;
        if (ratio != 100) {
            ratio = ratio.toFixed(1);
        }
        matchedtxt.firstChild.nodeValue = "Matched: " + ratio + "%";
    }
]]>
</script>
<rect x="0" y="0" width="${width}" height="${height}" fill="url(#background)"  />
<text id="title" x="${xtitle}" y="${ytoptext}">Flame Graph</text>
<text id="details" x="${xdetails}" y="${ybottomtext}"> </text>
<text id="unzoom" x="${xunzoom}" y="${ytoptext}" class="hide">Reset Zoom</text>
<text id="search" x="${xsearch}" y="${ytoptext}">Search</text>
<text id="ignorecase" x="${xignorecase}" y="${ytoptext}">ic</text>
<text id="matched" x="${xmatched}" y="${ybottomtext}"> </text>
<g id="frames">
${frames}</g>
</svg>)###";

void FlameGraph::Report()
{
    GenSortedStacks();
    MergeStacks();

    size_t depthMax = 0;
    for (auto stack : m_sortedStacks) {
        auto depth = std::get<0>(stack).size();
        if (depth > depthMax) {
            depthMax = depth;
        }
    }

    int times = std::get<3>(m_mergedStackNodes.back());
    /* Margins on both sides of the image. */
    float sideMargin = 10;
    /* Margins on top of the image. */
    float topMargin = m_fontSize * 3;
    /* Margins on bottom of the image. */
    float bottomMargin = m_fontSize * 2 + 10;
    float imageHeight = static_cast<float>(depthMax) * m_frameHeight + topMargin + bottomMargin;

    std::ofstream ofs(m_cfg.output);
    if (ofs.fail()) {
        fprintf(stderr, "error: Create flame graph '%s' failed.\n", m_cfg.output.c_str());
        return;
    }

    std::vector<std::pair<std::string, std::string>> replaceMap = {
        { "${width}", std::to_string(m_imageWidth) },
        { "${height}", std::to_string(imageHeight) },
        { "${fontsize}", std::to_string(m_fontSize) },
        { "${fontwidth}", std::to_string(m_fontWidth) },
        /* Font size of the title. */
        { "${titlesize}", std::to_string(m_fontSize + 5) },
        { "${xpad}", std::to_string(sideMargin) },
        /* Horizontal position of the title. */
        { "${xtitle}", std::to_string(m_imageWidth / 2) },
        { "${xdetails}", std::to_string(sideMargin) },
        { "${xunzoom}", std::to_string(sideMargin) },
        /* Horizontal position of the 'search' text. */
        { "${xsearch}", std::to_string(m_imageWidth - sideMargin - 100) },
        /* Horizontal position of the 'ignore case' text. */
        { "${xignorecase}", std::to_string(m_imageWidth - sideMargin - 16) },
        /* Horizontal position of the matched text. */
        { "${xmatched}", std::to_string(m_imageWidth - sideMargin - 100) },
        /* Vertical position from the top of the image. */
        { "${ytoptext}", std::to_string(m_fontSize * 2) },
        /* Vertical position from the bottom of the image. */
        { "${ybottomtext}", std::to_string(imageHeight - (bottomMargin / 2)) },
        { "${frames}", GenFrames(sideMargin, bottomMargin, times, imageHeight) }
    };
    ofs << ReplaceVariable(m_svgTemplate, replaceMap);

    ofs.close();
}

void FlameGraph::GenSortedStacks()
{
    std::map<Stack, Times> stackTimes;
    for (auto data : m_samplesData) {
        Stack stack = { "all" };
        for (auto it = data.stackTrace.rbegin(); it != data.stackTrace.rend(); it++) {
            stack.push_back(*it);
        }

        if (stackTimes.find(stack) == stackTimes.end()) {
            stackTimes[stack] = 0;
        }

        stackTimes[stack] += static_cast<Times>(data.num);
    }

    m_sortedStacks = std::vector<std::pair<Stack, Times>>(stackTimes.begin(), stackTimes.end());
    auto comp = [](const std::pair<Stack, Times> &a, const std::pair<Stack, Times> &b) {
        return std::get<0>(a) < std::get<0>(b);
    };
    std::sort(m_sortedStacks.begin(), m_sortedStacks.end(), comp);
}

void FlameGraph::MergeStacks()
{
    std::map<std::pair<FuncName, StackDepth>, StartTime> unresolvedNodes;
    Stack &last = std::get<0>(m_sortedStacks[0]);
    for (size_t i = 0; i < last.size(); i++) {
        unresolvedNodes.insert(std::make_pair(std::make_pair(last[i], i), 0));
    }

    Times times = std::get<1>(m_sortedStacks[0]);
    for (size_t i = 1; i < m_sortedStacks.size(); i++) {
        std::vector<std::string> &cur = std::get<0>(m_sortedStacks[i]);
        size_t sameSize = 0;
        while ((sameSize < last.size()) && (sameSize < cur.size()) && (cur[sameSize] == last[sameSize])) {
            sameSize++;
        }

        for (auto j = sameSize; j < last.size(); j++) {
            auto it = unresolvedNodes.find(std::make_pair(last[j], j));
            m_mergedStackNodes.push_back({last[j], j, it->second, times});
            unresolvedNodes.erase(it);
        }

        for (auto j = sameSize; j < cur.size(); j++) {
            unresolvedNodes.insert(std::make_pair(std::make_pair(cur[j], j), times));
        }

        times += std::get<1>(m_sortedStacks[i]);
        last = cur;
    }

    for (auto t : unresolvedNodes) {
        m_mergedStackNodes.push_back({std::get<0>(t.first), std::get<1>(t.first), t.second, times});
    }
    unresolvedNodes.clear();
}

std::string FlameGraph::ReplaceVariable(const std::string &ori,
    const std::vector<std::pair<std::string, std::string>> &replaceMap)
{
    std::string result = ori;
    for (auto r : replaceMap) {
        auto pos = result.find(r.first);
        while (pos != std::string::npos) {
            result.replace(pos, r.first.size(), r.second);
            pos = result.find(r.first, pos + r.second.size());
        }
    }

    return result;
}

std::string FlameGraph::GenFrames(float sideMargin, float bottomMargin, int times, float imageHeight)
{
    /* The total width of the sample is the width of the image minus the spacing between the two sides. */
    float widthPerTime = (m_imageWidth - sideMargin * 2) / static_cast<float>(times);
    float minWidthTime = m_minWidth / widthPerTime;

    std::stringstream ss;
    /* Sets the floating-point printing precision to 2 decimal places. */
    ss << std::setiosflags(std::ios::fixed) << std::setprecision(2);

    /* Initializes the random number seed, which is used to generate random warm colors later. */
    srand(static_cast<unsigned int>(time(0)));
    for (auto node : m_mergedStackNodes) {
        auto func = std::get<0>(node);
        auto depth = std::get<1>(node);
        auto startTime = std::get<2>(node);
        auto endTime = std::get<3>(node);
        if (static_cast<float>(endTime - startTime) < minWidthTime) {
            continue;
        }

        float left = sideMargin + static_cast<float>(startTime) * widthPerTime;
        float right = sideMargin + static_cast<float>(endTime) * widthPerTime;
        float bottom = imageHeight - bottomMargin - static_cast<float>(depth) * m_frameHeight;
        float top = bottom - m_frameHeight + 1;
        int samples = endTime - startTime;
        /* Percentage of this sample. */
        float pct = static_cast<float>((100.00 * samples) / times);

        /* HTML escape character table. */
        std::vector<std::pair<std::string, std::string>> htmlEsc = {
            { "&", "&amp;" },
            { "<", "&lt;" },
            { ">", "&gt;" },
            { "\"", "&quot;" },
            { "'", "&#39;" }
        };

        ss << "<g>\n";
        ss << "<title>" << ReplaceVariable(func, htmlEsc) << " (" << samples << " samples, " << pct << "%)</title>\n";

        auto warmColor = []() -> std::string {
            /* Red value of the warm RGB color. */
            int r = 205 + rand() % 50;
            /* Green value of the warm RGB color. */
            int g = rand() % 230;
            /* Blue value of the warm RGB color. */
            int b = rand() % 55;
            return "rgb(" + std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b) + ")";
        };
        ss << "<rect x = \"" << left << "\" y=\"" << top << "\" width=\"" << (right - left) <<
            "\" height=\"" << (bottom - top) << "\" fill=\"" << warmColor() << "\" rx=\"2\" ry=\"2\"/>\n";

        int chars = static_cast<int>((right - left) / (m_fontSize * m_fontWidth));
        /* Three is room for one char plus two dots. */
        std::string text = (chars >= 3) ? ((chars >= static_cast<int>(func.size()))
            ? func : func.substr(0, chars - 2) + "..") : "";

        /* Leave a margin of 3 to the left and bottom of the text */
        ss << "<text x=\"" << (left + 3) << "\" y=\"" << (bottom - 3) << "\">" <<
            ReplaceVariable(text, htmlEsc) << "</text>\n";
        ss << "</g>\n";
    }

    return ss.str();
}