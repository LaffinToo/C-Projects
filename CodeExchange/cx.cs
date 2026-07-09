using System;
using System.IO;
using System.Text;

namespace CodeExchange
{
    /// <summary>
    /// CodeExchange (cx) v3.0.0 — C# Core Implementation
    /// Optimized stack-allocation structure utilizing modern file access windows.
    /// </summary>
    class Program
    {
        private const int IoBufferSize = 16384;
        private static readonly char[] HexDigits = "0123456789ABCDEF".ToCharArray();

        private static void EmitRawBuffer(StreamWriter output, byte[] buffer, int len)
        {
            if (len == 0) return;
            byte crc = 0;
            for (int i = 0; i < len; i++)
            {
                crc ^= buffer[i];
            }
            
            string payload = Encoding.ASCII.GetString(buffer, 0, len);
            output.Write(payload);
            output.Write('|');
            output.Write(HexDigits[(crc >> 4) & 0x0F]);
            output.Write(HexDigits[crc & 0x0F]);
            output.Write('\n');
        }

        private static int FlushRun(byte[] buffer, int bufLen, byte charByte, int count, StreamWriter output, int lineLength)
        {
            if (count == 0) return bufLen;

            byte[] normalSeq = charByte switch
            {
                10 => Encoding.ASCII.GetBytes("\\n"),
                9  => Encoding.ASCII.GetBytes("\\t"),
                13 => Encoding.ASCII.GetBytes("\\r"),
                92 => Encoding.ASCII.GetBytes("\\\\"),
                _  => new byte[] { charByte }
            };

            int normalWidth = normalSeq.Length;
            int maxAllowed = lineLength > 4 ? lineLength - 4 : lineLength;

            while (count > 0)
            {
                int chunk = Math.Min(count, 65535);
                int rawCost = normalWidth * chunk;
                int rleCost = chunk > 255 ? (6 + normalWidth) : (4 + normalWidth);

                if (lineLength > 0 && (bufLen + rleCost > maxAllowed))
                {
                    if (bufLen > 0)
                    {
                        EmitRawBuffer(output, buffer, bufLen);
                        bufLen = 0;
                        continue;
                    }
                }

                if (rleCost < rawCost)
                {
                    string marker = chunk > 255 ? $"/z{chunk:X4}" : $"/x{chunk:X2}";
                    byte[] markerBytes = Encoding.ASCII.GetBytes(marker);
                    
                    Array.Copy(markerBytes, 0, buffer, bufLen, markerBytes.Length);
                    bufLen += markerBytes.Length;
                    Array.Copy(normalSeq, 0, buffer, bufLen, normalSeq.Length);
                    bufLen += normalSeq.Length;
                    
                    count -= chunk;
                }
                else
                {
                    int processNow = chunk;
                    if (lineLength > 0)
                    {
                        int spaceLeft = Math.Max(0, maxAllowed - bufLen);
                        int maxRaw = spaceLeft / normalWidth;
                        if (maxRaw == 0)
                        {
                            EmitRawBuffer(output, buffer, bufLen);
                            bufLen = 0;
                            continue;
                        }
                        processNow = Math.Min(processNow, maxRaw);
                    }

                    for (int i = 0; i < processNow; i++)
                    {
                        Array.Copy(normalSeq, 0, buffer, bufLen, normalSeq.Length);
                        bufLen += normalSeq.Length;
                    }
                    count -= processNow;
                }

                if (lineLength > 0 && bufLen >= maxAllowed)
                {
                    EmitRawBuffer(output, buffer, bufLen);
                    bufLen = 0;
                }
            }
            return bufLen;
        }

        private static void EncodeFile(StreamWriter output, string filePath, int lineLength)
        {
            if (!File.Exists(filePath)) return;

            output.Write($"===== FILE: {filePath} - LL:{lineLength} ====\n");

            byte globalCrc = 0;
            byte[] lineBuffer = new byte[4096];
            int bufLen = 0;
            
            int runCount = 0;
            byte currentByte = 0;
            byte[] ioBuf = new byte[IoBufferSize];

            using (FileStream fs = File.OpenRead(filePath))
            {
                int bytesRead;
                while ((bytesRead = fs.Read(ioBuf, 0, ioBuf.Length)) > 0)
                {
                    for (int i = 0; i < bytesRead; i++)
                    {
                        byte b = ioBuf[i];
                        globalCrc ^= b;

                        if (runCount == 0)
                        {
                            currentByte = b;
                            runCount = 1;
                        }
                        else if (b == currentByte)
                        {
                            runCount++;
                        }
                        else
                        {
                            bufLen = FlushRun(lineBuffer, bufLen, currentByte, runCount, output, lineLength);
                            currentByte = b;
                            runCount = 1;
                        }
                    }
                }
            }

            if (runCount > 0) bufLen = FlushRun(lineBuffer, bufLen, currentByte, runCount, output, lineLength);
            if (bufLen > 0) EmitRawBuffer(output, lineBuffer, bufLen);

            output.Write($"===== ENDFILE: {filePath} - FCS:{globalCrc:X2} ====\n");
        }

        static void Main(string[] args)
        {
            if (args.Length < 2) return;

            if (args[0] == "-e")
            {
                int lineLimit = 80;
                int fileIdx = 1;
                if (args.Length > 2 && args[1] == "-l")
                {
                    int.TryParse(args[2], out lineLimit);
                    fileIdx = 3;
                }

                using (StreamWriter stdout = new StreamWriter(Console.OpenStandardOutput(), Encoding.ASCII))
                {
                    stdout.AutoFlush = true;
                    stdout.Write("CodeExchange format - xc format v3 (Single-Pass Stream Architecture)\n\n");
                    for (int i = fileIdx; i < args.Length; i++)
                    {
                        EncodeFile(stdout, args[i], lineLimit);
                    }
                    stdout.Write("===== EOF\n");
                }
            }
        }
    }
}
