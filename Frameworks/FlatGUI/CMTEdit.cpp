#include "CMTEdit.h"

#include "FlatGUI/messages_multi_controls.hpp"

#include <fstream>


CMTEdit::CMTEdit(CFrame* frame, const CRect& size)
:CViewContainer (size),
IControlListener(),
IDependency(),
m_frame(frame)
{
	bMouseOn = false;
	HL1 = HL2 = HL3 = 0;

	whereOld = CPoint(-1.0,-1.0);

	m_scaledFontFactory = new ScaledFontFactory("Tahoma_Small");
	m_configSlider = new ConfigSlider();
	m_configSwitch = new ConfigSwitch();

	const CCoord width = size.getWidth();
	const CCoord height = size.getHeight();
	const CCoord fontSize = height/25.0;
	const CCoord fontSizeWas = 16;

	rBox1 = CRect(0,0,width/5.0,fontSize);
	rBox1.offset(0.0,fontSize+4.0);

	pBox1 = new CSliderNano(rBox1, this, 2,	*m_configSlider,kNoFrame|kHorizontal);

	this->addView(pBox1,rBox1,true);

	rBox2 = CRect(0,0,width/5.0,fontSize);
	rBox2.offset(width-width/5.0-4.0,height-rBox2.getHeight()-4.0);
	CSimpleOnOff* pSwitch1 = new CSimpleOnOff(rBox2, this, 3, *m_configSwitch);
	pSwitch1->setString32("prova"); 

	this->addView (pSwitch1);

	bLoadNoSave = true;
}

CMTEdit::~CMTEdit()
{
	m_scaledFontFactory = 0;
	m_configSlider = 0;
	m_configSwitch = 0;
	pBox1 = 0;
	pSwitch1 = 0;
}

CMouseEventResult CMTEdit::onMouseDown (CPoint& where, const CButtonState& buttons)
{	
	if(rBox1.pointInside(where))
	{
		return pBox1->onMouseDown(where,buttons);
	}
	if(rBox2.pointInside(where))
	{
		return kMouseEventNotHandled;
	}

	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	if(rClose.pointInside(where))
	{
		getFrame ()->setModalView(NULL);

	}else if(rMTEditLoad.pointInside(where)){

		bLoadNoSave = true;
		runFileSelector (true);	

	}else if(rMTEditSave.pointInside(where)){

		bLoadNoSave = false;
		runFileSelector (false);		
	}

	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

CMouseEventResult CMTEdit::onMouseMoved (CPoint& where, const CButtonState& buttons)
{	
	if(rBox1.pointInside(where))
	{
		return pBox1->onMouseMoved (where,buttons);		
	}
	if(rBox2.pointInside(where))
	{
		return kMouseEventNotHandled;
	}

	HL1 = HL2 = HL3 = 0;

	if(whereOld != where)
	{

		if(rClose.pointInside(where))
		{
			HL1 = 45;		
		}
		else if(rMTEditLoad.pointInside(where))
		{
			HL2 = 45;		
		}
		else if(rMTEditSave.pointInside(where))
		{
			HL3 = 45;		
		}
		invalidRect(rClose);
		invalidRect(rMTEditLoad);
		invalidRect(rMTEditSave);
		setDirty();

		whereOld = where;
		invalidRect(size);
		return kMouseMoveEventHandledButDontNeedMoreEvents;
	}

	return kMouseEventNotHandled;
}

CMouseEventResult CMTEdit::onMouseUp (CPoint& where, const CButtonState& buttons)
{	
	if(rBox1.pointInside(where))
	{
		return pBox1->onMouseUp (where,buttons);		
	}
	if(rBox2.pointInside(where))
	{
		return kMouseEventNotHandled;		
	}
	return kMouseEventNotHandled;
}

void CMTEdit::runFileSelector (bool load)
{
	CNewFileSelector* selector = CNewFileSelector::create (getFrame(), (load)? CNewFileSelector::kSelectFile : CNewFileSelector::kSelectSaveFile);
	if (selector)
	{
		// We don't need to set an initial directory right now, because Windows and macOS have sensible defaults for
		// user document files. When we implement a "NUSofting Data/Microtuning" directory we should set this.
		//UTF8StringPtr path = "C:\Documents and Settings\user\Desktop";
		//selector->setInitialDirectory (path);
		selector->addFileExtension (CFileExtension ("ALL", "*"));
		selector->addFileExtension (CFileExtension ("TUN", "tun"));
		selector->setDefaultExtension (CFileExtension ("SCL", "scl"));	
		selector->setTitle((load)? "Load File" : "Save File");
		selector->run (this);
		selector->forget ();
	}
}
CMessageResult CMTEdit::notify (CBaseObject* sender, IdStringPtr message)
{
	if (message == CNewFileSelector::kSelectEndMessage)
	{
		CNewFileSelector* sel = dynamic_cast<CNewFileSelector*>(sender);
		if (sel && sel->getNumSelectedFiles() > 0)
		{
			UTF8StringPtr path = sel->getSelectedFile(0);

			if(bLoadNoSave)
			{
				std::ifstream infile(path);

				if(infile.bad() || infile.fail())
				{
					const char* lines[] = { "Could not open microtuning scale file!", path };
					MessAgeBox(m_frame, "Error", lines, 2);
					return kMessageNotified;
				}

				std::string filePath = path;
				unsigned int ptr1 = filePath.find_last_of("/\\"); // Search for a forward slash or backslash
				scaleName = &filePath.c_str()[ptr1+1];
				unsigned int ptr2 = scaleName.find_last_of('.');
				scaleName.resize(ptr2);

				for(int i = 0; i < 128; ++i) valuesTun[i] = 0.0f;

				int indexNote = 0;
				std::string line;
				while (std::getline(infile, line)) // parser of the VAZ values in the TUN file
				{
					if(strstr(line.c_str(),"note"))
					{
						if(indexNote>127) break;
						const char* ptr = strchr(line.c_str(), '=');
						valuesTun[indexNote++] = atof(ptr+1); // assuming no space after '='						
					}
				}
			}
			else // save
			{
				std::ofstream onfile(path);

				if(onfile.bad() || onfile.fail())
				{
					const char* lines[] = { "Could not save microtuning scale file!", path };
					MessAgeBox(m_frame, "Error", lines, 2);
					return kMessageNotified;
				}

				for(int i = 0; i < 128; ++i) valuesTun[i] = 0.0f; // to be changed by GUI

				std::string string1 = "; VAZ Plus/AnaMark softsynth tuning file";
				std::string string2 = ";";
				std::string string3 = "; VAZ Plus section";
				std::string string4 = "[Tuning]";	
				onfile << string1+"\n";
				onfile << string2+"\n";
				onfile << string3+"\n";
				onfile << string4+"\n";
				for(int i = 0; i < 128; ++i)
				{
					onfile << "note =" << static_cast<int>(valuesTun[i]) << "\n";
				}
				onfile.close();
			}
			changed(kSetTunNewFile);

		}//if (sel) end
		// do anything with the selected files here
		return kMessageNotified;
	}
	//return parent::notify (sender, message); ??
	return kMessageUnknown;
}

IdStringPtr CMTEdit::kSetTunNewFile = "kSetTunNewFile";
