using System;
using System.Text;
using System.Runtime.InteropServices;

public class CELLRecvStream
{
    private byte[] _buff = null;
    private int _readPos = 0;

    public CELLRecvStream(IntPtr pData, int nLen)
    {
        byte[] buff = new byte[nLen];
        Marshal.Copy(pData, buff, 0, nLen);
        _buff = buff;
        _readPos = 0;
    }

    private bool canRead(int n)
    {
        if (n < 1 || n > _buff.Length)
            return false;
        return _buff.Length - _readPos >= n;
    }

    private void pop(int n)
    {
        _readPos += n;
    }

    //有符号
    public sbyte ReadInt8()
    {
        sbyte n = 0;
        if (canRead(1))
        {
            n = (sbyte)_buff[_readPos];
            pop(1);
        }
        return n;
    }

    public Int16 ReadInt16()
    {
        Int16 n = 0;
        if (canRead(2))
        {
            n = BitConverter.ToInt16(_buff, _readPos);
            pop(2);
        }
        return n;
    }

    public Int32 ReadInt32()
    {
        Int32 n = 0;
        if (canRead(4))
        {
            n = BitConverter.ToInt32(_buff, _readPos);
            pop(4);
        }
        return n;
    }

    public Int64 ReadInt64()
    {
        Int64 n = 0;
        if (canRead(8))
        {
            n = BitConverter.ToInt64(_buff, _readPos);
            pop(8);
        }
        return n;
    }

    //无符号
    public byte ReadUInt8()
    {
        byte n = 0;
        if (canRead(1))
        {
            n = _buff[_readPos];
            pop(1);
        }
        return n;
    }

    public UInt16 ReadUInt16()
    {
        UInt16 n = 0;
        if (canRead(2))
        {
            n = BitConverter.ToUInt16(_buff, _readPos);
            pop(2);
        }
        return n;
    }

    public UInt32 ReadUInt32()
    {
        UInt32 n = 0;
        if (canRead(4))
        {
            n = BitConverter.ToUInt32(_buff, _readPos);
            pop(4);
        }
        return n;
    }

    public UInt64 ReadUInt64()
    {
        UInt64 n = 0;
        if (canRead(8))
        {
            n = BitConverter.ToUInt64(_buff, _readPos);
            pop(8);
        }
        return n;
    }
    //浮点数
    public float ReadFloat()
    {
        float n = 0.0f;
        if (canRead(4))
        {
            n = BitConverter.ToSingle(_buff, _readPos);
            pop(4);
        }
        return n;
    }

    public double ReadDouble()
    {
        double n = 0.0;
        if (canRead(8))
        {
            n = BitConverter.ToDouble(_buff, _readPos);
            pop(8);
        }
        return n;
    }
    //TODO
    public byte[] ReadBytes()
    {
        return null;
    }

    public string ReadString()
    {
        Int32 len = (Int32)ReadUInt32();
        string str = string.Empty;
        if (len > 1 && canRead(len))
        {
            str = Encoding.UTF8.GetString(_buff, _readPos, len);
            pop(len);
        }
        return str;
    }

    public Int32[] ReadInt32s()
    {
        UInt32 len = ReadUInt32();
        Int32[] data = new Int32[len];
        for (int n = 0; n < len; n++)
        {
            data[n] = ReadInt32();
        }
        return data;
    }

    //
    public UInt16 ReadNetLength()
    {
        return ReadUInt16();
    }

    public NetCMD ReadNetCMD()
    {
        return (NetCMD)ReadUInt16();
    }
}
