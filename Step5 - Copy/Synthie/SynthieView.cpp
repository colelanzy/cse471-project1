
// SynthieView.cpp : implementation of the CSynthieView class
//

#include "stdafx.h"
#include "Synthie.h"
#include "SynthieView.h"
#include <cmath>
#include "audio/DirSoundSource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

short RangeBound(double d)
{
    if(d < -32768)
        return -32768;
    else if(d > 32767)
        return 32767;

    return (short)d;
}

// CSynthieView

CSynthieView::CSynthieView()
{
    m_audiooutput = true;
    m_fileoutput = false;
	m_synthesizer.SetNumChannels(NumChannels());
	m_synthesizer.SetSampleRate(SampleRate());
}

CSynthieView::~CSynthieView()
{
}


BEGIN_MESSAGE_MAP(CSynthieView, CWnd)
	ON_WM_PAINT()
	ON_COMMAND(ID_GENERATE_FILEOUTPUT, &CSynthieView::OnGenerateFileoutput)
	ON_UPDATE_COMMAND_UI(ID_GENERATE_FILEOUTPUT, &CSynthieView::OnUpdateGenerateFileoutput)
	ON_COMMAND(ID_GENERATE_AUDIOOUTPUT, &CSynthieView::OnGenerateAudiooutput)
	ON_UPDATE_COMMAND_UI(ID_GENERATE_AUDIOOUTPUT, &CSynthieView::OnUpdateGenerateAudiooutput)
	ON_COMMAND(ID_GENERATE_1000HZTONE, &CSynthieView::OnGenerate1000hztone)
	ON_COMMAND(ID_GENERATE_SYNTHESIZER, &CSynthieView::OnGenerateSynthesizer)
	ON_COMMAND(ID_FILE_OPENSCORE, &CSynthieView::OnFileOpenscore)
END_MESSAGE_MAP()



// CSynthieView message handlers

BOOL CSynthieView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), 
		NULL);

	return TRUE;
}

void CSynthieView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CWnd::OnPaint() for painting messages
}


void CSynthieView::OnGenerate1000hztone()
{
	// Call to open the generator output
	if(!GenerateBegin())
		return;

	short audio[2];

	double freq = 1000;
	double duration = 5;

	for(double time=0.;  time < duration;  time += 1. / SampleRate())
	{                 
		audio[0] = short(3200 * sin(time * 2 * PI * freq));
		audio[1] = audio[0];

		GenerateWriteFrame(audio);

		// The progress control
		if(ProgressAbortCheck())
			break;
	}


	// Call to close the generator output
	GenerateEnd();
}




//
// Name :        CSynthieView::GenerateBegin()
// Description : This function opens an audio file for output as
//               m_wave.  Be sure to call EndGenerate() when done.
// Returns :     true if successful...
//

bool CSynthieView::GenerateBegin()
{
	// 
	// Waveform storage
	//

	m_waveformBuffer.Start(NumChannels(), SampleRate());

	if(m_fileoutput)
	{
	  if(!OpenGenerateFile(m_wave))
		 return false;
	}

	ProgressBegin(this);

	if(m_audiooutput)
	{
	  m_soundstream.SetChannels(NumChannels());
	  m_soundstream.SetSampleRate(int(SampleRate()));

	  m_soundstream.Open(((CSynthieApp *)AfxGetApp())->DirSound());
	}

	return true;
}

//
// Name :        CSynthieView::GenerateWriteFrame()
// Description : Write a frame of output to the current generation device.
//

void CSynthieView::GenerateWriteFrame(short *p_frame)
{
    m_waveformBuffer.Frame(p_frame);

    if(m_fileoutput)
        m_wave.WriteFrame(p_frame);

    if(m_audiooutput)
        m_soundstream.WriteFrame(p_frame);
}


//
// Name :        CSynthieView::GenerateEnd()
// Description : End the generation process.
//

void CSynthieView::GenerateEnd()
{
    m_waveformBuffer.End();

    if(m_fileoutput)
        m_wave.close();

    if(m_audiooutput)
        m_soundstream.Close();

    ProgressEnd(this);
}

//
// Name :        CSynthieView::OpenGenerateFile()
// Description : This function opens the audio file for output.
// Returns :     true if successful...
//

bool CSynthieView::OpenGenerateFile(CWaveOut &p_wave)
{
   p_wave.NumChannels(NumChannels());
   p_wave.SampleRate(SampleRate());

	static WCHAR BASED_CODE szFilter[] = L"Wave Files (*.wav)|*.wav|All Files (*.*)|*.*||";

	CFileDialog dlg(FALSE, L".wav", NULL, 0, szFilter, NULL);
	if(dlg.DoModal() != IDOK)
      return false;

   p_wave.open(dlg.GetPathName());
   if(p_wave.fail())
      return false;

   return true;
}

void CSynthieView::OnGenerateFileoutput()
{
	m_fileoutput = !m_fileoutput;
}

void CSynthieView::OnUpdateGenerateFileoutput(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_fileoutput);	
}


void CSynthieView::OnGenerateAudiooutput()
{
	m_audiooutput = !m_audiooutput;
}

void CSynthieView::OnUpdateGenerateAudiooutput(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_audiooutput);	
}



void CSynthieView::OnGenerateSynthesizer()
{
	// Path to the anthem.wav file
	std::string filepath = "path_to_your_file/anthem.wav";

	// Initialize DirSoundSource to load the file
	DirSoundSource soundSource;

	// Attempt to load the audio file
	if (soundSource.Open(filepath.c_str()))
	{
		AfxMessageBox(L"anthem.wav file loaded successfully!");

		// Check if the file contains audio frames and then play
		if (soundSource.GetFrameCount() > 0)
		{
			// Start playback
			soundSource.StartPlay();

			// Wait until the playback is finished
			while (soundSource.IsPlaying())
			{
				Sleep(100); // Small delay to keep checking
			}

			AfxMessageBox(L"Playback finished!");
		}
		else
		{
			AfxMessageBox(L"Error: The audio file is empty or couldn't be read.");
		}

		// Close the file after playback
		soundSource.Close();
	}
	else
	{
		AfxMessageBox(L"Failed to load anthem.wav. Please check the file path.");
	}
}


void CSynthieView::OnFileOpenscore()
{
	static WCHAR BASED_CODE szFilter[] = L"Score files (*.score)|*.score|All Files (*.*)|*.*||";

	CFileDialog dlg(TRUE, L".score", NULL, 0, szFilter, NULL);
	if (dlg.DoModal() != IDOK)
		return;

	m_synthesizer.OpenScore(dlg.GetPathName());
}
