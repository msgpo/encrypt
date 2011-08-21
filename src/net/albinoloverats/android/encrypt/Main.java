package net.albinoloverats.android.encrypt;

import java.io.File;
import java.util.Iterator;
import java.util.Set;

import net.albinoloverats.android.encrypt.R;
import net.albinoloverats.android.encrypt.Encrypt.Status;
import android.app.Activity;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.DialogInterface.OnCancelListener;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnKeyListener;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.OnItemSelectedListener;

import com.lamerman.FileDialog;

public class Main extends Activity
{
    private static final int PROGRESS_DIALOG = 0;

    private Set<String> cipherNames;
    private Set<String> hashNames;

    private ProgressDialog progressDialog;
    private ProgressThread progressThread;

    private boolean encrypting = true;
    private String filenameIn;
    private String filenameOut;
    private String hash;
    private String cipher;
    private String password;

    @Override
    public void onCreate(final Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        // setup the file chooser button
        final Button fChooser = (Button)findViewById(R.id.button_file);
        fChooser.setOnClickListener(new OnClickListener()
        {
            @Override
            public void onClick(final View v)
            {
                final Intent intent = new Intent(Main.this.getBaseContext(), FileDialog.class);
                intent.putExtra(FileDialog.START_PATH, "/sdcard");
                Main.this.startActivityForResult(intent, FileDialog.REQUEST_LOAD);
            }
        });

        // setup the file output chooser button
        final Button oChooser = (Button)findViewById(R.id.button_output);
        oChooser.setOnClickListener(new OnClickListener()
        {
            @Override
            public void onClick(final View v)
            {
                final Intent intent = new Intent(Main.this.getBaseContext(), FileDialog.class);
                intent.putExtra(FileDialog.START_PATH, "/sdcard");
                Main.this.startActivityForResult(intent, FileDialog.REQUEST_SAVE);
            }
        });

        // setup the hash and crypto spinners
        final Spinner cSpinner = (Spinner)findViewById(R.id.spin_crypto);
        final ArrayAdapter<CharSequence> cipherSpinAdapter = new ArrayAdapter<CharSequence>(this, android.R.layout.simple_spinner_item);
        cipherSpinAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        cSpinner.setAdapter(cipherSpinAdapter);
        cSpinner.setOnItemSelectedListener(new OnItemSelectedListener()
        {
            @Override
            public void onItemSelected(final AdapterView<?> parent, final View view, final int position, final long id)
            {
                int i = 0;
                for (final Iterator<String> iterator = cipherNames.iterator(); iterator.hasNext(); i++)
                {
                    if (position > 0 && i == position - 1)
                        cipher = (String)iterator.next();
                    else
                    {
                        if (position == 0)
                            cipher = null;
                        iterator.next();
                    }
                }
                checkEnableEncryptButton();
            }
            @Override
            public void onNothingSelected(final AdapterView<?> parent)
            {
                ;
            }
        });
        cSpinner.setEnabled(false);

        final Spinner hSpinner = (Spinner)findViewById(R.id.spin_hash);
        final ArrayAdapter<CharSequence> hashSpinAdapter = new ArrayAdapter<CharSequence>(this, android.R.layout.simple_spinner_item);
        hashSpinAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        hSpinner.setAdapter(hashSpinAdapter);
        hSpinner.setOnItemSelectedListener(new OnItemSelectedListener()
        {
            @Override
            public void onItemSelected(final AdapterView<?> parent, final View view, final int position, final long id)
            {
                int i = 0;
                for (final Iterator<String> iterator = hashNames.iterator(); iterator.hasNext(); i++)
                {
                    if (i > 0 && i == position - 1)
                        hash = (String)iterator.next();
                    else
                    {
                        if (position == 0)
                            hash = null;
                        iterator.next();
                    }
                }
                checkEnableEncryptButton();
            }
            @Override
            public void onNothingSelected(final AdapterView<?> parent)
            {
                ;
            }
        });
        hSpinner.setEnabled(false);

        cipherNames = AlgorithmNames.getCipherAlgorithmNames();
        hashNames = AlgorithmNames.getHashAlgorithmNames();
        cipherSpinAdapter.add(getString(R.string.choose_cipher));
        for (final String s : cipherNames)
            cipherSpinAdapter.add(s);
        hashSpinAdapter.add(getString(R.string.choose_hash));
        for (final String s : hashNames)
            hashSpinAdapter.add(s);

        // get reference to password text box
        final EditText pEntry = (EditText)findViewById(R.id.text_password);
        pEntry.setOnKeyListener(new OnKeyListener()
        {
            @Override
            public boolean onKey(final View v, final int keyCode, final KeyEvent event)
            {
                final String p = ((EditText)findViewById(R.id.text_password)).getText().toString();
                if (p.length() == 0)
                    password = null;
                else
                    password = p;
                checkEnableEncryptButton();
                return false;
            }
        });
        pEntry.setEnabled(false);

        // get reference to encrypt/decrypt button
        final Button encutton = (Button)findViewById(R.id.button_go);
        encutton.setOnClickListener(new OnClickListener()
        {
            @Override
            public void onClick(final View v)
            {
                showDialog(PROGRESS_DIALOG);
            }
        });
        encutton.setEnabled(false);

        fChooser.requestFocus();
    }

    @Override
    public boolean onCreateOptionsMenu(final Menu menu)
    {
        getMenuInflater().inflate(R.menu.menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(final MenuItem item)
    {
        switch (item.getItemId())
        {
            case R.id.menu_about:
                aboutDialog();
                break;
        }
        return true;
    }

    /*
     * show about dialog
     */
    private void aboutDialog()
    {
        final Dialog dialog = new Dialog(this);
        dialog.setContentView(R.layout.about);
        dialog.setTitle(getString(R.string.app_name) + " " + getString(R.string.version));
        ((ImageView)dialog.findViewById(R.id.about_image)).setImageResource(R.drawable.icon);
        ((TextView)dialog.findViewById(R.id.about_text)).setText(getString(R.string.shpeel) + "\n" + getString(R.string.copyright) + "\n" + getString(R.string.url));
        dialog.show();
    }

    public synchronized void onActivityResult(final int requestCode, int resultCode, final Intent data)
    {
        if (resultCode == Activity.RESULT_OK)
        {
            switch (requestCode)
            {
                case FileDialog.REQUEST_LOAD:
                    filenameIn = data.getStringExtra(FileDialog.RESULT_PATH);
                    ((Button)findViewById(R.id.button_file)).setText(filenameIn);
                    break;
                case FileDialog.REQUEST_SAVE:
                    filenameOut = data.getStringExtra(FileDialog.RESULT_PATH);
                    ((Button)findViewById(R.id.button_output)).setText(filenameOut);
                    break;
            }
            if (filenameIn != null && filenameOut != null)
            {
                ((EditText)findViewById(R.id.text_password)).setEnabled(true);
    
                final Spinner cSpinner = (Spinner)findViewById(R.id.spin_crypto);
                final Spinner hSpinner = (Spinner)findViewById(R.id.spin_hash);
    
                final Button encButton = (Button)findViewById(R.id.button_go);
    
                if (Encrypt.fileEncrypted(new File(filenameIn)))
                {
                    encrypting = false;
                    encButton.setText(R.string.decrypt);
                    hSpinner.setEnabled(false);
                    cSpinner.setEnabled(false);
                }
                else
                {
                    encrypting = true;
                    encButton.setText(R.string.encrypt);
                    hSpinner.setEnabled(true);
                    cSpinner.setEnabled(true);
                    hSpinner.requestFocus();
                }
            }
        }
        else if (resultCode == Activity.RESULT_CANCELED)
        {
            ;
        }
    }

    protected Dialog onCreateDialog(final int id)
    {
        switch (id)
        {
            case PROGRESS_DIALOG:
                progressDialog = new ProgressDialog(Main.this);
                progressDialog.setMessage(getString(R.string.please_wait));
                progressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
                progressDialog.setOnCancelListener(new OnCancelListener()
                {
                    @Override
                    public void onCancel(final DialogInterface dialog)
                    {
                        progressThread.interrupt();
                    }
                });
                return progressDialog;
            default:
                return null;
        }
    }

    @Override
    protected void onPrepareDialog(final int id, final Dialog dialog)
    {
        switch(id)
        {
            case PROGRESS_DIALOG:
                progressDialog.setProgress(0);
                progressThread = new ProgressThread(handler);
                progressThread.start();
        }
    }

    final Handler handler = new Handler()
    {
        public void handleMessage(final Message msg)
        {
            if (msg.arg1 > 0)
            {
                progressDialog.setMax(msg.arg1);
                progressDialog.setProgress(msg.arg2);
            }
            else if (msg.arg1 < 0)
            {
                dismissDialog(PROGRESS_DIALOG);
                Toast.makeText(getApplicationContext(), (String)msg.obj, Toast.LENGTH_LONG).show();
            }
        }
    };

    private void checkEnableEncryptButton()
    {
        if ((encrypting && hash != null && cipher != null && password != null) || (!encrypting && password != null))
            findViewById(R.id.button_go).setEnabled(true);
        else
            findViewById(R.id.button_go).setEnabled(false);
    }

    private class ProgressThread extends Thread
    {
        private Handler mHandler;

        private ProgressThread(final Handler h)
        {
            mHandler = h;
        }
       
        public void run()
        {
            final Encrypt encryptProcess;
            if (encrypting)
                encryptProcess = new Encrypt(new File(filenameIn), new File(filenameOut), password.getBytes(), hash, cipher);
            else
                encryptProcess = new Encrypt(new File(filenameIn), new File(filenameOut), password.getBytes());

            encryptProcess.start();

            Status status = null;
            do
            {
                try
                {
                    sleep(10);
                }
                catch (final InterruptedException e)
                {
                    encryptProcess.interrupt();
                }
                mHandler.sendMessage(mHandler.obtainMessage(0, (int)encryptProcess.getDecryptedSize(), (int)encryptProcess.getBytesProcessed()));

                status = encryptProcess.getStatus();
            }
            while (status == Status.NOT_STARTED || status == Status.RUNNING);
            final String finished;
            switch (status)
            {
                case CANCELLED:
                    finished = getString(R.string.cancelled);
                    break;
                case FAILED_IO:
                    finished = getString(R.string.failed_io);
                    break;
                case FAILED_ALGORITHM:
                    finished = getString(R.string.failed_algorithm) + " : " + status.getAdditional();
                    break;
                case FAILED_KEY:
                    finished = getString(R.string.failed_key);
                    break;
                case FAILED_DECRYPTION:
                    finished = getString(R.string.failed_decryption);
                    break;
                case FAILED_CHECKSUM:
                    finished = getString(R.string.failed_checksum);
                    break;
                default:
                    finished = getString(encrypting ? R.string.encryption_succeeded : R.string.decryption_succeeded);
                    break;
            }
            mHandler.sendMessage(mHandler.obtainMessage(0, -1, -1, finished));
        }
    }
}