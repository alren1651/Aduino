
void setup()
{
    Serial.begin(115200);     // Ch.0 - Upload & Test
    Serial.setTimeout(10);    // Serial Stream을 읽는 시간을 10 ms로 변경

    // ----------------------------------------------------------------------------- //
    Serial1.begin(115200);    // Ch.1 - MsgBoard
    Serial1.setTimeout(10);   // Serial Stream을 읽는 시간을 10 ms로 변경
    // ----------------------------------------------------------------------------- //
    Serial2.begin(115200);    // Ch.2 - sLory
    Serial2.setTimeout(10);   // Serial Stream을 읽는 시간을 10 ms로 변경

    // ----------------------------------------------------------------------------- //
    pinMode(22, OUTPUT);
    pinMode(23, OUTPUT);
    pinMode(24, OUTPUT);
    pinMode(25, OUTPUT);
}

void loop()
{
    // If anything comes in Serial2 (sLory)
    if (Serial2.available())
    {
        // ============================================================================= //
        // Read Serial_Data
        String str_Recv = Serial2.readStringUntil('\n');
        // ----------------------------------------------------------------------------- //
        int i_PosSeparator = str_Recv.indexOf("/");
        // ----------------------------------------------------------------------------- //
        String str_WindSpd = str_Recv.substring(0, i_PosSeparator);
        int i_Cmd = str_Recv.substring(i_PosSeparator+1, str_Recv.length()).toInt();

        // ========================================================================= //
        // 풍속에 따라 Indicator Color 변경
        // ------------------------------------------------------------------------- //
        double d_WindSpd = str_WindSpd.toDouble();
        // Default Font Color : $C00 - White/Black/Transparency
        byte b_Color[8] = { 0x24, 0x00, 0x43, 0x00, 0x30, 0x00, 0x30, 0x00 };
        // ------------------------------------------------------------------------- //
        if (0 <= d_WindSpd && d_WindSpd <= 9.9)
        {
            // Normal : $C03 - Green/Black/Transparency
            b_Color[6] = 0x33;
        }
        else if (10.0 <= d_WindSpd && d_WindSpd <= 12.9)
        {
            // Attention : $C02 - Yellow/Black/Transparency
            b_Color[6] = 0x32;
        }
        else if (13.0 <= d_WindSpd && d_WindSpd <= 14.9)
        {
            // Warning : $C10 - White/Red/Transparency
            b_Color[4] = 0x31;
            b_Color[6] = 0x30;
        }
        else
        {
            // Alart : $C01 - Red/Black/Transparency
            b_Color[6] = 0x31;
        }

        // ========================================================================= //
        // RST=1,LNE=1,YSZ=1,TXT=$F00$C00●●●,
        // ------------------------------------------------------------------------- //
        byte b_MsgLn1[68] = {
            0x52, 0x00, 0x53, 0x00, 0x54, 0x00, 0x3D, 0x00, 0x31, 0x00, 0x2C, 0x00,
            0x4C, 0x00, 0x4E, 0x00, 0x45, 0x00, 0x3D, 0x00, 0x31, 0x00, 0x2C, 0x00,
            0x59, 0x00, 0x53, 0x00, 0x5A, 0x00, 0x3D, 0x00, 0x31, 0x00, 0x2C, 0x00,
            0x54, 0x00, 0x58, 0x00, 0x54, 0x00, 0x3D, 0x00, 0x24, 0x00, 0x46, 0x00, 0x30, 0x00, 0x30, 0x00,
            0x24, 0x00, 0x43, 0x00, 0x30, 0x00, 0x30, 0x00,
            0xCF, 0x25, 0xCF, 0x25, 0xCF, 0x25, 0x2C, 0x00
        };
        // Change $C00
        memmove(b_MsgLn1 + 52, b_Color, 8);

        // ------------------------------------------------------------------------- //
        // LNE=2,YSZ=1,TXT=$F00$C00
        // ------------------------------------------------------------------------- //
        byte b_MsgLn2[48] = {
            0x4C, 0x00, 0x4E, 0x00, 0x45, 0x00, 0x3D, 0x00, 0x32, 0x00, 0x2C, 0x00,
            0x59, 0x00, 0x53, 0x00, 0x5A, 0x00, 0x3D, 0x00, 0x31, 0x00, 0x2C, 0x00,
            0x54, 0x00, 0x58, 0x00, 0x54, 0x00, 0x3D, 0x00, 0x24, 0x00, 0x46, 0x00, 0x30, 0x00, 0x30, 0x00,
            0x24, 0x00, 0x43, 0x00, 0x30, 0x00, 0x30, 0x00
        };
        // Change $C00
        memmove(b_MsgLn2 + 40, b_Color, 8);

        // ------------------------------------------------------------------------- //
        // $C00 m/s
        // ------------------------------------------------------------------------- //
        byte b_MsgEnd[16] = {
            0x24, 0x00, 0x43, 0x00, 0x30, 0x00, 0x30, 0x00, 0x20, 0x00, 0x6D, 0x00, 0x2F, 0x00, 0x73, 0x00
        };

        // ========================================================================= //
        // 풍속을 byte 배열로 변환
        // ------------------------------------------------------------------------- //
        int i_strLen = str_WindSpd.length();
        byte b_WindSpd[i_strLen] = {0,};
        str_WindSpd.getBytes(b_WindSpd, i_strLen+1);
        // ------------------------------------------------------------------------- //
        int i_bLen = i_strLen * 2;
        byte b_WindData[i_bLen] = {0,};
        // ------------------------------------------------------------------------- //
        for (int i=0; i < i_bLen; i+=2)
        {
            b_WindData[i] = b_WindSpd[i/2];
            b_WindData[i+1] = 0x00;
        }

        // ========================================================================= //
        // Tx_Command byte 생성
        int i_TxCmdLen = 132 + i_bLen;
        byte b_TxCmd[i_TxCmdLen + 6] = {0,};
        // ------------------------------------------------------------------------- //
        b_TxCmd[0] = 0x02;
        b_TxCmd[1] = 0x84;
        b_TxCmd[2] = lowByte(i_TxCmdLen);
        b_TxCmd[3] = highByte(i_TxCmdLen);

        // [Array] b_MsgLn1 + [Array] b_MsgLn2 + [Array] b_WindData + [Array] b_MsgEnd
        memmove(b_TxCmd+4, b_MsgLn1, 68);
        memmove(b_TxCmd+4+68, b_MsgLn2, 48);
        memmove(b_TxCmd+4+116, b_WindData, i_bLen);
        memmove(b_TxCmd+4+116+i_bLen, b_MsgEnd, 16);
        b_TxCmd[4] = 0x52;

        // ------------------------------------------------------------------------- //
        // Make the Command Footer
        byte ChkSum_ETX[2] = { 0x00, 0x03 };
        // Calculate the CheckSum
        for (int i=0; i<i_TxCmdLen+4; i++)
        {
            ChkSum_ETX[0] += b_TxCmd[i];
        }
        // ------------------------------------------------------------------------- //
        memmove(b_TxCmd+4+i_TxCmdLen, ChkSum_ETX, 2);

        // ========================================================================= //
        // Send Data
        Serial1.write(b_TxCmd, sizeof(b_TxCmd));

        // ========================================================================= //
        // Control DC sink 출력
        digitalWrite(i_Cmd, HIGH);
        delay(1000);
        digitalWrite(i_Cmd, LOW);

        // ========================================================================= //
/*
        for (int i=0; i<i_TxCmdLen+6; i++)
        {
            Serial.print(b_TxCmd[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
*/
    }
}
