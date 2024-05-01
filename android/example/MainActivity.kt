/**
 * MIT License
 *
 * Copyright 2024 PE Stanislav Yahniukov <pe@yahniukov.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the “Software”), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

package com.yahniukov.cryptodb

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.os.Environment
import android.provider.Settings
import android.view.View
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {
    private val DB_PATH = "/sdcard/Download/cryptodb"

    private var tv: TextView? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val version: CryptoDB.Version = CryptoDB.GetVersion()
        tv = findViewById<View>(R.id.tv) as TextView
        tv?.setText(String.format("Version: %d.%d.%d",
            version.major,
            version.minor,
            version.revision))

        if (Environment.isExternalStorageManager()) {
            cryptodbAndroidTest()
        } else {
            val intent = Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION)
            val uri = Uri.fromParts("package", packageName, null)
            intent.setData(uri)
            startActivity(intent)
            if (Environment.isExternalStorageManager())
                cryptodbAndroidTest()
        }
    }

    fun Char.repeat(count: Int): String = this.toString().repeat(count)

    private fun cryptodbAndroidTest() {
        val uniq_data_len = CryptoDB.GetUniqDataMaxLen()
        val uniq_data_str = '0'.repeat(uniq_data_len.toInt())
        val uniq_data = uniq_data_str.toByteArray()

        CryptoDB.SetDefaultOptions()

        var err: CryptoDB.Error = CryptoDB.Open(DB_PATH, uniq_data)
        if (err != CryptoDB.Error.OK) {
            tv?.setText(String.format("ERROR: Open(): %s",
                CryptoDB.ErrorToString(err)))
            return
        }

        err = CryptoDB.PutString("test_string", "test_string_val")
        if (err != CryptoDB.Error.OK) {
            tv?.setText(String.format("ERROR: PutString(): %s",
                CryptoDB.ErrorToString(err)))
            CryptoDB.Close()
            return
        }
        err = CryptoDB.PutInteger("test_int", 42)
        if (err != CryptoDB.Error.OK) {
            tv?.setText(String.format("ERROR: PutInteger(): %s",
                CryptoDB.ErrorToString(err)))
            CryptoDB.Close()
            return
        }
        err = CryptoDB.PutDouble("test_double", 42.42)
        if (err != CryptoDB.Error.OK) {
            tv?.setText(String.format("ERROR: PutDouble(): %s",
                CryptoDB.ErrorToString(err)))
            CryptoDB.Close()
            return
        }

        var getStringResult: CryptoDB.GetStringResult = CryptoDB.GetString(
            "test_string",
            "test_string_val".length * 2)
        if (getStringResult.err != CryptoDB.Error.OK ||
            getStringResult.`val` != "test_string_val") {
            tv?.setText(String.format("ERROR: GetString(): %s : %s",
                CryptoDB.ErrorToString(getStringResult.err), getStringResult.`val`))
            CryptoDB.Close()
            return
        }
        var getIntegerResult: CryptoDB.GetIntegerResult = CryptoDB.GetInteger("test_int")
        if (getIntegerResult.err != CryptoDB.Error.OK ||
            getIntegerResult.`val` != 42) {
            tv?.setText(String.format("ERROR: GetInteger(): %s : %d",
                CryptoDB.ErrorToString(getIntegerResult.err), getIntegerResult.`val`))
            CryptoDB.Close()
            return
        }
        var getDoubleResult: CryptoDB.GetDoubleResult = CryptoDB.GetDouble("test_double")
        if (getDoubleResult.err != CryptoDB.Error.OK ||
            getDoubleResult.`val` != 42.42) {
            tv?.setText(String.format("ERROR: GetDouble(): %s : %f",
                CryptoDB.ErrorToString(getDoubleResult.err), getDoubleResult.`val`))
            CryptoDB.Close()
            return
        }

        err = CryptoDB.Delete("test_string")
        if (err != CryptoDB.Error.OK) {
            tv?.setText(String.format("ERROR: Delete(): %s",
                CryptoDB.ErrorToString(err)))
            CryptoDB.Close()
            return
        }
        err = CryptoDB.Delete("test_int")
        if (err != CryptoDB.Error.OK) {
            tv?.setText(String.format("ERROR: Delete(): %s",
                CryptoDB.ErrorToString(err)))
            CryptoDB.Close()
            return
        }
        err = CryptoDB.Delete("test_double")
        if (err != CryptoDB.Error.OK) {
            tv?.setText(String.format("ERROR: Delete(): %s",
                CryptoDB.ErrorToString(err)))
            CryptoDB.Close()
            return
        }

        getStringResult = CryptoDB.GetString(
            "test_string",
            "test_string_val".length)
        getIntegerResult = CryptoDB.GetInteger("test_int")
        getDoubleResult = CryptoDB.GetDouble("test_double")

        if (getStringResult.err   == CryptoDB.Error.OK ||
            getIntegerResult.err  == CryptoDB.Error.OK ||
            getDoubleResult.err   == CryptoDB.Error.OK)
        {
            tv?.setText("ERROR: Delete() is not working correctly")
            CryptoDB.Close()
            return
        }

        CryptoDB.Close()
        err = CryptoDB.Destroy(DB_PATH)
        if (err != CryptoDB.Error.OK) {
            tv?.setText(String.format("ERROR: Destroy(): %s",
                CryptoDB.ErrorToString(err)))
            return
        }

        tv?.setText("PASS")
    }
}