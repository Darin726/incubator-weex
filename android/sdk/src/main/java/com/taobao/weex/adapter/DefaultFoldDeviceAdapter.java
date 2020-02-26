/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
package com.taobao.weex.adapter;

import android.content.Context;
import android.os.Build;
import android.support.annotation.NonNull;
import android.text.TextUtils;

import com.taobao.weex.utils.WXLogUtils;

import java.util.ArrayList;
import java.util.List;

public class DefaultFoldDeviceAdapter implements IWXFoldDeviceAdapter {

    private final static String TAG = "TBDeviceUtils";
    private final static List<String> HUAWEI_FOLD_DEVICES;
    private final static String HUAWEI_BRAND = "HUAWEI";
    private final static String SAMSUNG_BRAND = "samsung";

    static {
        HUAWEI_FOLD_DEVICES = new ArrayList<String>() {{
            add("unknownRLI");
            add("HWTAH");
            add("MRX-AL09");
            add("HWMRX");
            add("TAH-AN00m");
            add("HWTAH-C");
            add("RHA-AN00m");
            add("unknownRHA");
        }};
    }

    /**
     * 折叠屏判断
     *
     * @return
     */
    @Override
    public boolean isFoldDevice() {

        if (SAMSUNG_BRAND.equalsIgnoreCase(Build.BRAND)) {
            if (TextUtils.equals("SM-F9000", Build.MODEL)) {
                return true;
            }
        }

        if (HUAWEI_BRAND.equalsIgnoreCase(Build.BRAND) && HUAWEI_FOLD_DEVICES.contains(Build.DEVICE)) {
            WXLogUtils.e(TAG, "is HUAWEI Fold Device");
            return true;
        }

        return false;
    }

    /**
     * 华为折叠屏设备判断
     */
    @Override
    public boolean isMateX() {
        if (HUAWEI_BRAND.equalsIgnoreCase(Build.BRAND) && HUAWEI_FOLD_DEVICES.contains(Build.DEVICE)) {
            WXLogUtils.e(TAG, "is HUAWEI Fold Device");
            return true;
        }

        return false;
    }

    /**
     * 三星Galaxy Fopld
     */
    @Override
    public boolean isGalaxyFold() {

        if (SAMSUNG_BRAND.equalsIgnoreCase(Build.BRAND) && "SM-F9000".equalsIgnoreCase(Build.MODEL)) {
            return true;
        }

        return false;
    }

}
