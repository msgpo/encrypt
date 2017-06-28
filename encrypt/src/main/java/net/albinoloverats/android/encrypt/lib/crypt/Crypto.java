/*
 * encrypt ~ a simple, modular, (multi-OS) encryption utility
 * Copyright © 2005-2015, albinoloverats ~ Software Development
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

package net.albinoloverats.android.encrypt.lib.crypt;

import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.PowerManager;
import android.support.v4.app.NotificationCompat;

import gnu.crypto.hash.IMessageDigest;

import java.io.ByteArrayOutputStream;
import java.io.Closeable;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import net.albinoloverats.android.encrypt.lib.misc.Convert;

public abstract class Crypto extends Service implements Runnable
{
	protected static final long[] HEADER = { 0x3697de5d96fca0faL, 0xc845c2fa95e2f52dL, Version.CURRENT.magicNumber };

	protected static final int BLOCK_SIZE = 1024;

	protected InputStream source;
	protected OutputStream output;

	protected String path;
	protected String name;
	protected String cipher;
	protected String hash;
	protected String mode;
	protected String mac;
	protected byte[] key;

	protected boolean raw = false;

	public Status status;
	public final Progress current = new Progress();
	public final Progress total = new Progress();

	protected int blockSize;
	protected boolean compressed = false;
	protected boolean directory = false;
	protected boolean follow_links = false;

	protected Version version = Version.CURRENT;

	protected IMessageDigest checksum;

	private Thread process;
	private Thread notification;
	private PowerManager.WakeLock wakeLock;

	private String actionTitle;

	@Override
	public void run()
	{
		try
		{
			process();
		}
		catch (final CryptoProcessException e)
		{
			status = e.code;
		}
		catch (final InterruptedException e)
		{
			status = Status.CANCELLED;
		}
		finally
		{
			releaseWakeLock();
		}
	}

	abstract protected void process() throws InterruptedException, CryptoProcessException;

	@Override
	public int onStartCommand(final Intent intent, final int flags, final int startId)
	{
		status = Status.INIT;

		final Class<?> clas = (Class<?>)intent.getSerializableExtra("class");
		final int action = intent.getIntExtra("action", 0);
		final int wait = intent.getIntExtra("wait", 0);
		final int icon = intent.getIntExtra("icon", 0);

		if (intent.getBooleanExtra("key_file", false))
			setKey(intent.getStringExtra("key"));
		else
			key = intent.getByteArrayExtra("key");

		actionTitle = getString(action);

		final NotificationCompat.Builder notificationBuilder = new NotificationCompat.Builder(getBaseContext());
		notificationBuilder.setContentTitle(actionTitle);
		notificationBuilder.setContentText(getString(wait));
		notificationBuilder.setSmallIcon(icon);

		final Intent notificationIntent = new Intent(this, clas);
		notificationIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
		notificationBuilder.setContentIntent(PendingIntent.getActivity(this, 0, notificationIntent, 0));

		if (status == Status.INIT)
		{
			/* start en/decryption process */
			final PowerManager powerManager = (PowerManager)getSystemService(POWER_SERVICE);
			wakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, actionTitle);
			wakeLock.acquire();
			process = new Thread(this);
			process.start();
		}

		/* start a thread to keep ui updated */
		notification = new Thread()
		{
			@Override
			public void run()
			{
				do
				{
					try
					{
						sleep(10);
						if (isInterrupted())
							status = Status.CANCELLED;
						if (status == Status.INIT)
							continue;
						sendNotifiationUpdate(notificationBuilder);
					}
					catch (final InterruptedException e)
					{
						status = Status.CANCELLED;
					}
				}
				while (status == Status.RUNNING);
				sendNotifiationUpdate(notificationBuilder);
			}
		};
		notification.start();

		return START_STICKY;
	}

	@Override
	public void onDestroy()
	{
		if (status == Status.INIT || status == Status.RUNNING)
		{
			notification.interrupt();
			process.interrupt();
		}
		releaseWakeLock();
		super.onDestroy();
	}

	private void releaseWakeLock()
	{
		if (wakeLock != null && wakeLock.isHeld())
		{
			wakeLock.release();
			wakeLock = null;
		}
	}

	@Override
	public IBinder onBind(final Intent arg0)
	{
		return null;
	}

	public static boolean fileEncrypted(final String path)
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
				int err = in.read(header, 0, header.length);
				if (err < 0 || Convert.longFromBytes(header) != HEADER[i])
					return false;
			}
			return true;
		}
		catch (final IOException ignored)
		{
			return false; // either the file doesn't exists or we can't read it for decrypting
		}
		finally
		{
			closeIgnoreException(in);
		}
	}

	protected static void closeIgnoreException(final Closeable c)
	{
		try
		{
			if (c != null)
				c.close();
		}
		catch (final IOException ignored)
		{
		}
	}

	private void setKey(final String k)
	{
		FileInputStream f = null;
		ByteArrayOutputStream b = new ByteArrayOutputStream();
		try
		{
			final File file = new File(k);
			f = new FileInputStream(file);
			key = new byte[(int)file.length()];
			f.read(key);
		}
		catch (final IOException e)
		{
			status = Status.FAILED_KEY;
		}
		finally
		{
			closeIgnoreException(f);
			closeIgnoreException(b);
		}
	}

	private void sendNotifiationUpdate(final NotificationCompat.Builder notificationBuilder)
	{
		final Intent intent = new Intent();
		intent.setAction(actionTitle);
		intent.putExtra("current.offset", current.offset);
		intent.putExtra("current.size", current.size);
		intent.putExtra("total.offset", total.offset);
		intent.putExtra("total.size", total.size);
		intent.putExtra("status", status.name());
		sendBroadcast(intent);

		notificationBuilder.setContentText(status == Status.INIT || status == Status.RUNNING ? total.offset + "/" + total.size : status.toString());
		int pct = 0;
		if (status == Status.SUCCESS)
			pct = 100;
		else if (current.size > 0)
			pct = (int)(100 * current.offset / current.size);
		notificationBuilder.setProgress(100, pct, false);
		((NotificationManager)getSystemService(Context.NOTIFICATION_SERVICE)).notify(0, notificationBuilder.build());
	}
}