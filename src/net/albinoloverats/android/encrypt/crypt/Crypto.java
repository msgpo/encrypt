/*
 * encrypt ~ a simple, modular, (multi-OS) encryption utility
 * Copyright © 2005-2012, albinoloverats ~ Software Development
 * email: encrypt@albinoloverats.net
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

package net.albinoloverats.android.encrypt.crypt;

import gnu.crypto.hash.IMessageDigest;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import net.albinoloverats.android.encrypt.misc.Convert;

public abstract class Crypto extends Thread implements Runnable
{
    protected static final long HEADER_VERSION_201108 = 0x72761df3e497c983L;
    protected static final long HEADER_VERSION_201110 = 0xbb116f7d00201110L;
    protected static final long HEADER_VERSION_201211 = 0x51d28245e1216c45L;
    protected static final long HEADER_VERSION_NEXT = 0xa5a5a5a5a5a5a5a5L;
    protected static final long[] HEADER = { 0x3697de5d96fca0faL, 0xc845c2fa95e2f52dL, HEADER_VERSION_NEXT };

    protected static final int BLOCK_SIZE = 1024;

    protected InputStream source;
    protected OutputStream output;

    protected String path;
    protected String cipher;
    protected String hash;
    protected byte[] key;

    public Status status = Status.INIT;
    public Progress current = new Progress();
    public Progress total = new Progress();

    protected int blocksize;
    protected boolean compressed = true;
    protected boolean directory = false;

    protected IMessageDigest checksum;

    @Override
    public void run()
    {
        try
        {
            process();
        }
        catch (final Exception e)
        {
            status = e.code;
        }
    }

    abstract protected void process() throws Exception;

    public static boolean fileEncrypted(final String path) throws IOException
    {
        final File f = new File(path);
        if (f.isDirectory())
            return false;
        
        FileInputStream in = null;
        try
        {
            in = new FileInputStream(f);
            final byte[] header = new byte[Long.SIZE / Byte.SIZE];
            for (int i = 0; i < 1; i++)
            {
                in.read(header, 0, header.length);
                if (Convert.longFromBytes(header) != HEADER[i])
                    return false;
            }
            return true;
        }
        finally
        {
            if (in != null)
                in.close();
        }
    }

}
