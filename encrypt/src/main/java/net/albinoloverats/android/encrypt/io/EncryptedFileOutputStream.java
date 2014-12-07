/*
 * encrypt ~ a simple, modular, (multi-OS) encryption utility
 * Copyright © 2005-2014, albinoloverats ~ Software Development
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

package net.albinoloverats.android.encrypt.io;

import gnu.crypto.cipher.IBlockCipher;
import gnu.crypto.hash.IMessageDigest;
import gnu.crypto.mode.IMode;
import gnu.crypto.mode.ModeFactory;
import gnu.crypto.util.PRNG;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.channels.FileChannel;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Map;

import net.albinoloverats.android.encrypt.crypt.CryptoUtils;
import net.albinoloverats.android.encrypt.crypt.XIV;
import net.albinoloverats.android.encrypt.misc.Convert;

public class EncryptedFileOutputStream extends FileOutputStream
{
    private final FileOutputStream stream;

    private IMode cipher;

    private byte[] buffer = null;
    private int blockSize = 0;
    private int[] offset = { 0, 0 };

    private boolean open = true;

    public EncryptedFileOutputStream(final File file) throws FileNotFoundException
    {
        super(file);
        stream = new FileOutputStream(file);
    }

    public IMessageDigest encryptionInit(final String cipher, final String hash, final String mode, final byte[] key, final XIV ivType) throws NoSuchAlgorithmException, InvalidKeyException, IOException
    {
        IMessageDigest h = CryptoUtils.getHashAlgorithm(hash);
        final IBlockCipher c = CryptoUtils.getCipherAlgorithm(cipher);
        blockSize = c.defaultBlockSize();
        this.cipher = ModeFactory.getInstance(mode, c, blockSize);
        h.update(key, 0, key.length);
        final byte[] keySource = h.digest();
        final Map<String, Object> attributes = new HashMap<String, Object>();
        final int keyLength = CryptoUtils.getCipherAlgorithmKeySize(cipher) / Byte.SIZE;
        final byte[] keyOutput = new byte[keyLength];
        System.arraycopy(keySource, 0, keyOutput, 0, keyLength < keySource.length ? keyLength : keySource.length);
        attributes.put(IBlockCipher.KEY_MATERIAL, keyOutput);
        attributes.put(IBlockCipher.CIPHER_BLOCK_SIZE, blockSize);
        attributes.put(IMode.STATE, IMode.ENCRYPTION);
        h.reset();
        h.update(keySource, 0, keySource.length);
        final byte[] iv = new byte[ivType != XIV.BROKEN ? blockSize : keyLength];
        switch (ivType)
        {
            case BROKEN:
            case SIMPLE:
                System.arraycopy(h.digest(), 0, iv, 0, iv.length);
                break;
            case RANDOM:
                PRNG.nextBytes(iv);
                stream.write(iv);
                break;
        }
        attributes.put(IMode.IV, iv);
        this.cipher.init(attributes);
        buffer = new byte[blockSize];
        return h;
    }

    @Override
    public void close() throws IOException
    {
        if (!open)
            return;

        if (cipher != null)
        {
            final int[] remainder = { 0, blockSize - offset[0] };
            final byte[] x = new byte[remainder[1]];
            PRNG.nextBytes(x);
            System.arraycopy(x, 0, buffer, offset[0], remainder[1]);
            final byte[] eBytes = new byte[blockSize];
            cipher.update(buffer, 0, eBytes, 0);
            stream.write(eBytes);
        }
        stream.flush();
        stream.close();
        open = false;
    }

    @Override
    protected void finalize() throws IOException
    {
        close();
        super.finalize();
    }

    @Override
    public FileChannel getChannel()
    {
        return stream.getChannel();
    }

    @Override
    public void write(final byte[] bytes) throws IOException
    {
        if (cipher == null)
        {
            stream.write(bytes);
            return;
        }
        final int[] remainder = { bytes.length, blockSize - offset[0] };
        offset[1] = 0;
        while (remainder[0] > 0)
        {
            if (remainder[0] < remainder[1])
            {
                System.arraycopy(bytes, offset[1], buffer, offset[0], remainder[0]);
                offset[0] += remainder[0];
                return;
            }
            System.arraycopy(bytes, offset[1], buffer, offset[0], remainder[1]);
            final byte[] eBytes = new byte[blockSize];
            cipher.update(buffer, 0, eBytes, 0);
            stream.write(eBytes);
            offset[0] = 0;
            buffer = new byte[blockSize];
            offset[1] += remainder[1];
            remainder[0] -= remainder[1];
            remainder[1] = blockSize - offset[0];
        }
    }

    @Override
    public void write(final byte[] b, final int off, final int len) throws IOException
    {
        final byte[] bytes = new byte[len];
        System.arraycopy(b, off, bytes, 0, len);
        write(bytes);
    }

    @Override
    public void write(final int b) throws IOException
    {
        write(Convert.toBytes((byte)(b & 0x000000FF)));
    }
}
