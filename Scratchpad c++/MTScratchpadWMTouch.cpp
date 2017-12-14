// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// MTScratchpadWMTouch application.
// Description:
//  Inside the application window, user can draw using multiple fingers
//  at the same time. The trace of each finger is drawn using different
//  color. The primary finger trace is always drawn in black, and the
//  remaining traces are drawn by rotating through the following colors:
//  red, blue, green, magenta, cyan and yellow.
//
// Purpose:
//  This sample demonstrates handling of the multi-touch input inside
//  a Win32 application using WM_TOUCH window message:
//  - Registering a window for multi-touch using RegisterTouchWindow,
//    IsTouchWindow.
//  - Handling WM_TOUCH messages and unpacking their parameters using
//    GetTouchInputInfo and CloseTouchInputHandle; reading touch contact
//    data from the TOUCHINPUT structure.
//  - Unregistering a window for multi-touch using UnregisterTouchWindow.
//  In addition, the sample also shows how to store and draw strokes
//  entered by user, using the helper classes CStroke and
//  CStrokeCollection.
//
// MTScratchpadWMTouch.cpp : Defines the entry point for the application.



// Windows header files
#include <windows.h>
#include <windowsx.h>

// C RunTime header files
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <assert.h>
#define ASSERT assert

// Application header files
#include "resource.h"
#include "Stroke.h"
#define MAX_LOADSTRING 100


// C++ STD header files
#include <string>
#include <map>
#include <vector>
#include <cmath>
#include <experimental/filesystem>
#include <sys/stat.h>

// MTCircGR API
#include "CircGR.h"
#include "GestureParser.h"
#include "tinyxml.h"
#include "tinystr.h"

// Namespaces
using namespace std;
namespace fs = std::experimental::filesystem;
namespace M = MTCircGR;



// Global Variables:
HINSTANCE g_hInst;                              // Current module instance
WCHAR g_wszTitle[MAX_LOADSTRING];               // The title bar text
WCHAR g_wszWindowClass[MAX_LOADSTRING];         // The main window class name
CStrokeCollection g_StrkColFinished;            // Finished strokes, the finger has been lifted
CStrokeCollection g_StrkColDrawing;             // Strokes that are currently being drawn


//*******************************************
//		CircGR Related Global Variables		*
//*******************************************
M::PointMap pointMap = M::PointMap();
int countPoints = 0;
int localPoints = 0;
int fingers = 0;
const int CANDIDATE_CLASSIFY_ONCOUNT = 64;
string templateGestureSetPath = "GestureSet/TemplateGestures/";
int numOfTemplates = 0;


bool modeTemplate = true;
bool holdGesture = true;
bool recognitionOn = true;
int gestureCount = 0;
int gestureTries = 1;
int captureWindowCount = 0;
int testInstanceCount = 0;
int candidateCount = 0;
bool dollarMatchOn = true;

// Holds information about the previous point recived on a certain touch id by touch form.
// Used for filtering purposes.
//map<int, TOUCHINPUT&> prevPoint = map<int, TOUCHINPUT&>(); 

// Used to implement filtering. Point will be filterd if it does not change by more than specified
// delta.
double FILTER_DELTA = 1;

// Setting this to true makes all touch events pop up on the log window.
bool DISPLAY_TOUCH_DATA = true;

// If set to true, input map is cleared after every window of points is classified rather than
// adding them to the map.
bool dropWindows = false;

// Holds the gestures that define classess. 
vector<M::Gesture> Templates;

// The amount of training Samples to collect.
int TRAINING_SAMPLES_TO_COLLECT = 10;

// The amount of training samples collected thus far.
int Training_SAMPLES_COLLECTED = 0;

// used to find the next gesture to train
int trainingPointer = 0;



// for test modes, number of times no classification is returned
int numNoClass = 0;

// Mode Flags

string expectedGesture = "";
//GestureWindowPerformance windowPerformance;   // CHECK OUT FOR FINDING ACCURACY
bool modeTest = false;
bool modeTraining = false;
bool modeExperiment = false;
string lastClassification = "";
M::Gesture lastCandidate;

// Set CircGR as Recognizer
//enum Recognizers { CircGR, OneDollar, PDollar, RawFeatures };
//Recognizers activeRecognizer;
//M::Recognizer recognizer;
M::CircGR recognizer;



string expectedStr = "";




#pragma region MTCIRCGR HELPER FUNCTIONS


// **********************************************
// *		MTCircGR API HELPER FUNCTIONS		*
// **********************************************

void printCircGRBanner()
{
	printf("=================================\n");
	printf("  ____ _           ____ ____	\n");
	printf(" / ___(_)_ __ ___ / ___|  _ \\	\n");
	printf("| |   | | '__/ __| |  _| |_) |	\n");
	printf("| |___| | | | (__| |_| |  _ <	\n");
	printf(" \\____|_|_|  \\___|\\____|_| \\_\\	\n");
	printf("=================================\n");
}



#pragma endregion



#pragma region GESTURE RECOGNITION


// Sets CircGR as gesture recognizer
void setRecognizer()
{
	printf("Setting CircGR as the gesture recognizer \n");
	M::CircGR recognizer = M::CircGR::CircGR(false);			// MODIFY BOOL FLAG FOR VERBOSE
}

// Loads templates by parsing gesture from xml and adding gesture object to Templates vector
void loadTemplates()
{

	printf("Loading Template gestures: \n\n");

	M::Gesture tg;

	
	for (auto & p : fs::directory_iterator(templateGestureSetPath))
	{
		if (fs::path(p).extension() == ".xml")
		{
			tg = M::GestureParser::parseGesture(fs::path(p).string().c_str(), M::Gesture::eGestureType::Template, false);

			// Add template gesture to recognizer
			Templates.push_back(tg);

			// Output gesture info
			cout << fs::path(p).filename() << ", ";

			numOfTemplates++;

		}
	}
	cout << endl;

}


// Sets templates that were loaded and parsed to the recognizer
void setTemplates()
{
	printf("\nA total of [%d] templates were successfully set \n", numOfTemplates);
	recognizer.SetTemplates(Templates);
}


void initRecognizer()
{
	setRecognizer();

	loadTemplates();

	setTemplates();
}


// Runs gesture recognition algorithm for an inputted gesture
// against the template gestures
void runRecognition()
{
	localPoints = 0;

	string classification = "";

	M::Gesture inputGesture = M::Gesture(pointMap, "Candidate", M::Gesture::eGestureType::Candidate);
	
	classification = recognizer.Classify(inputGesture);
	
	cout << "Input classified as: " + classification << endl;
}



#pragma endregion



#pragma region DRAWING & WM_TOUCH HELPERS



// **********************************************
// *	Drawing and WM_TOUCH helpers			*
// **********************************************
// Returns color for the newly started stroke.
// in:
//      bPrimaryContact     boolean, whether the contact is the primary contact
// returns:
//      COLORREF, color of the stroke
COLORREF GetTouchColor(bool bPrimaryContact)
{
    static int g_iCurrColor = 0;    // Rotating secondary color index
    static COLORREF g_arrColor[] =  // Secondary colors array
    {
        RGB(255, 0, 0),             // Red
        RGB(0, 255, 0),             // Green
        RGB(0, 0, 255),             // Blue
        RGB(0, 255, 255),           // Cyan
        RGB(255, 0, 255),           // Magenta
        RGB(255, 255, 0)            // Yellow
    };

    COLORREF color;
    if (bPrimaryContact)
    {
        // The primary contact is drawn in black.
        color = RGB(0,0,0);         // Black
    }
    else
    {
        // Take current secondary color.
        color = g_arrColor[g_iCurrColor];

        // Move to the next color in the array.
        g_iCurrColor = (g_iCurrColor + 1) % (sizeof(g_arrColor)/sizeof(g_arrColor[0]));
    }

    return color;
}



// Extracts contact point in client area coordinates (pixels) from a TOUCHINPUT
// structure. TOUCHINPUT structure uses 100th of a pixel units.
// in:
//      hWnd        window handle
//      ti          TOUCHINPUT structure (info about contact)
// returns:
//      POINT with contact coordinates
POINT GetTouchPoint(HWND hWnd, const TOUCHINPUT& ti)
{
    POINT pt;
    pt.x = ti.x / 100;
    pt.y = ti.y / 100;
    ScreenToClient(hWnd, &pt);
    return pt;
}



// Extracts contact ID from a TOUCHINPUT structure.
// in:
//      ti          TOUCHINPUT structure (info about contact)
// returns:
//      ID assigned to the contact
inline int GetTouchContactID(const TOUCHINPUT& ti)
{
    return ti.dwID;
}




#pragma endregion



#pragma region WM_TOUCH MESSAGE HANDLERS (INPUT ACQUISITION)

// ******************************************************
// *		Input Acquisition helper functions			*
// ******************************************************
void incPoints()
{
	countPoints++;
	localPoints++;
}

void cleanUp(HWND hWnd)
{
	// Invalidate scratchpad
	InvalidateRect(hWnd, NULL, TRUE);

	// clears relevant global variables
	cout << "Cleaning up [" << countPoints << "] total points" << endl;
	pointMap.Clear();
	localPoints = 0;
	countPoints = 0;
}

void clearScratchpad()
{
	// Remove colored strokes
	int i;
	for (i = 0; i < g_StrkColFinished.Count(); ++i)
	{
		g_StrkColFinished.Remove(i);
	}
	for (i = 0; i < g_StrkColDrawing.Count(); ++i)
	{
		g_StrkColDrawing.Remove(i);
	}
}


//bool FilterPoint(HWND hWnd, const TOUCHINPUT& ti)
//{
//	
//	if ((abs(GetTouchPoint(hWnd, ti).x - GetTouchPoint(hWnd,prevPoint[ti.dwID]).x) < FILTER_DELTA) && (abs(GetTouchPoint(hWnd, ti).y - GetTouchPoint(hWnd, prevPoint[ti.dwID]).y)) < FILTER_DELTA)
//	{
//		//LogWM(e, "Filtered");
//		return true;
//	}
//
//	return false;
//}

//void UpdatePrevious(const TOUCHINPUT& ti)
//{
//	if (prevPoint.find(ti.dwID) != prevPoint.end())			// prevPoint.ContainsKey(e.Id)
//		prevPoint[ti.dwID] = ti;
//	else
//		prevPoint.insert({ ti.dwID, ti });
//}

//void LogWM(const TOUCHINPUT& ti, string type)
//{
//	if (!DISPLAY_TOUCH_DATA)
//		return;
//
//	string isPrimary = (e.IsPrimaryContact) ? "*" : "^";
//	fLog.addLine("ID" + isPrimary + " {" + type + "}=" + +e.Id + "[" + e.LocationX + ","
//		+ e.LocationY + "] Cx,y " + e.ContactX + "," + e.ContactY + "] t="
//		+ e.Time + " .");
//
//}




// **********************************************
// *		WM_TOUCH message handlers			*
// **********************************************

// Handler for touch-down message.
// Starts a new stroke and assigns a color to it.
// in:
//      hWnd        window handle
//      ti          TOUCHINPUT structure (info about contact)
void OnTouchDownHandler(HWND hWnd, const TOUCHINPUT& ti)
{

    // Extract contact info: point of contact and ID
    POINT pt = GetTouchPoint(hWnd, ti);
    int iCursorId = GetTouchContactID(ti);

    // We have just started a new stroke, which must have an ID value unique
    // among all the strokes currently being drawn. Check if there is a stroke
    // with the same ID in the collection of the strokes in drawing.
    ASSERT(g_StrkColDrawing.FindStrokeById(iCursorId) == -1);

	// Get color
	COLORREF c = GetTouchColor((ti.dwFlags & TOUCHEVENTF_PRIMARY) != 0);

	// If color is the primary color (black) then print banner
	if (c == RGB(0, 0, 0))
	{
		cout << "\n***********************************\n" << endl;
		cout << "*\tSTARTING RECOGNITION	  *\n" << endl;
		cout << "***********************************\n" << endl;
	}

    // Create new stroke, add point and assign a color to it.
    CStroke* pStrkNew = new CStroke;
    pStrkNew->Add(pt);
    pStrkNew->SetColor(c);
    pStrkNew->SetId(iCursorId);

    // Add new stroke to the collection of strokes in drawing.
    g_StrkColDrawing.Add(pStrkNew);



	//CIRCGR CODE FOR DOWN HANDLER 
	fingers++;
	incPoints();

	POINT touchpt = GetTouchPoint(hWnd, ti);

	M::Point p = M::Point(touchpt.x, touchpt.y, ti.dwID, ti.dwTime);
	pointMap.Add(p);

	//UpdatePrevious(ti);


}



// Handler for touch-move message.
// Adds a point to the stroke in drawing and draws new stroke segment.
// in:
//      hWnd        window handle
//      ti          TOUCHINPUT structure (info about contact)
void OnTouchMoveHandler(HWND hWnd, const TOUCHINPUT& ti)
{
    // Extract contact info: contact ID
    int iCursorId = GetTouchContactID(ti);

    // Find the stroke in the collection of the strokes in drawing.
    int iStrk = g_StrkColDrawing.FindStrokeById(iCursorId);
    ASSERT((iStrk >= 0) && (iStrk < g_StrkColDrawing.Count()));

    // Extract contact info: contact point
    POINT pt;
    pt = GetTouchPoint(hWnd, ti);

    // Add contact point to the stroke
    g_StrkColDrawing[iStrk]->Add(pt);

    // Partial redraw: only the last line segment
    HDC hDC = GetDC(hWnd);
    g_StrkColDrawing[iStrk]->DrawLast(hDC);
    ReleaseDC(hWnd, hDC);



	//CIRCGR CODE FOR MOVE HANDLER

	incPoints();
	//LogWM(e, "M");
	POINT touchpt = GetTouchPoint(hWnd, ti);
	M::Point p = M::Point(touchpt.x, touchpt.y, ti.dwID, ti.dwTime);
	pointMap.Add(p);

	if ((localPoints >= (CANDIDATE_CLASSIFY_ONCOUNT)) && !modeTraining) //capture gestures continously when in training, rather than in parts
		runRecognition();




}



// Handler for touch-up message.
// Finishes the stroke and moves it to the collection of finished strokes.
// in:
//      hWnd        window handle
//      ti          TOUCHINPUT structure (info about contact)
void OnTouchUpHandler(HWND hWnd, const TOUCHINPUT& ti)
{
    // Extract contact info: contact ID
    int iCursorId = GetTouchContactID(ti);

    // Find the stroke in the collection of the strokes in drawing.
    int iStrk = g_StrkColDrawing.FindStrokeById(iCursorId);
    ASSERT((iStrk >= 0) && (iStrk < g_StrkColDrawing.Count()));

    // Add this stroke to the collection of finished strokes.
    g_StrkColFinished.Add(g_StrkColDrawing[iStrk]);

    // Remove this stroke from the collection of strokes in drawing.
    g_StrkColDrawing.Remove(iStrk);


	
	//CIRCGR CODE FOR UP HANDLER
	incPoints();
	//LogWM(e, "U");
	POINT touchpt = GetTouchPoint(hWnd, ti);
	M::Point p = M::Point(touchpt.x, touchpt.y, ti.dwID, ti.dwTime);
	pointMap.Add(p);


	fingers--;

	if ((localPoints >= (fingers * CANDIDATE_CLASSIFY_ONCOUNT) && !modeTraining) || fingers == 0)
	{
		runRecognition();

		if (fingers == 0)
		{
			cleanUp(hWnd);
		}
		
	}
		
	
}


#pragma endregion



#pragma region APPLICATION FRAMEWORK



// **********************************************
// *		Application framework				*
// **********************************************

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

// Win32 application main entry point function.
// This function is generated by Visual Studio app wizard.
// in:
//      hInstance       handle of the application instance
//      hPrevInstance   not used, always NULL
//      lpCmdLine       command line for the application, null-terminated string
//      nCmdShow        how to show the window
int APIENTRY wWinMain(HINSTANCE hInstance,
                     HINSTANCE /* hPrevInstance */,
                     LPWSTR    /* lpCmdLine */,
                     int       nCmdShow)
{

	// Allocates Output Console
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen("CON", "w", stdout);

	// Print welcome banner
	printCircGRBanner();

	// Initiate CircGR recognizer
	initRecognizer();


    MSG msg;

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, g_wszTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_MTSCRATCHPADWMTOUCH, g_wszWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    // Main message loop
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, NULL, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



// Registers the window class of the application.
// This function is generated by Visual Studio app wizard.
// This function and its usage are only necessary if you want this code
// to be compatible with Win32 systems prior to the 'RegisterClassEx'
// function that was added to Windows 95. It is important to call this function
// so that the application will get 'well formed' small icons associated
// with it.
// in:
//      hInstance       handle to the instance of the application
// returns:
//      class atom that uniquely identifies the window class
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = 0;
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = g_wszWindowClass;
    wcex.hIconSm        = 0;

    return RegisterClassEx(&wcex);
}




// Saves instance handle and creates main window
// This function is generated by Visual Studio app wizard; added code for
// registering the app window for multi-touch.
// In this function, we save the instance handle in a global variable,
// create and display the main program window, and also register the window
// for receiving multi-touch messages.
// in:
//      hInstance       handle to the instance of the application
//      nCmdShow        how to show the window
// returns:
//      boolean, succeeded or failed to create the window
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{

    HWND hWnd;

    g_hInst = hInstance; // Store instance handle in our global variable

    // Create the application window
    hWnd = CreateWindow(g_wszWindowClass, g_wszTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
    if (!hWnd)
    {
        return FALSE;
    }

    // Register application window for receiving multi-touch input. Use default settings.
    if (!RegisterTouchWindow(hWnd, 0))
    {
        MessageBox(hWnd, L"Cannot register application window for multi-touch input", L"Error", MB_OK);
        return FALSE;
    }
    ASSERT(IsTouchWindow(hWnd, NULL));

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);


    return TRUE;
}




// Processes messages for the main window:
//      WM_COMMAND  - process the application menu
//      WM_PAINT    - paint the main window
//      WM_TOUCH    - multi-touch message
//      WM_DESTROY  - post a quit message and return
// This function is generated by Visual Studio app wizard; added WM_PAINT, WM_TOUCH
// and WM_DESTROY code.
// in:
//      hWnd        window handle
//      message     message code
//      wParam      message parameter (message-specific)  Number of Inputs
//      lParam      message parameter (message-specific)
// returns:
//      the result of the message processing and depends on the message sent
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
        case WM_COMMAND:
            wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;

        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            // Full redraw: draw complete collection of finished strokes and
            // also all the strokes that are currently in drawing.
            g_StrkColFinished.Draw(hdc);
            g_StrkColDrawing.Draw(hdc);
            EndPaint(hWnd, &ps);
            break;

        // WM_TOUCH message handlers
        case WM_TOUCH:
            {
                // WM_TOUCH message can contain several messages from different contacts
                // packed together.
                // Message parameters need to be decoded:
                unsigned int numInputs = (unsigned int) wParam; // Number of actual per-contact messages
                TOUCHINPUT* ti = new TOUCHINPUT[numInputs]; // Allocate the storage for the parameters of the per-contact messages
                if (ti == NULL)
                {
                    break;
                }
                
				// Unpack message parameters into the array of TOUCHINPUT structures, each
                // representing a message for one single contact.
                if (GetTouchInputInfo((HTOUCHINPUT)lParam, numInputs, ti, sizeof(TOUCHINPUT)))
                {
                    // For each contact, dispatch the message to the appropriate message
                    // handler.
                    for (unsigned int i = 0; i < numInputs; ++i)
                    {
                        if (ti[i].dwFlags & TOUCHEVENTF_DOWN)
                        {
                            OnTouchDownHandler(hWnd, ti[i]);
                        }
                        else if (ti[i].dwFlags & TOUCHEVENTF_MOVE)
                        {
                            OnTouchMoveHandler(hWnd, ti[i]);
                        }
                        else if (ti[i].dwFlags & TOUCHEVENTF_UP)
                        {
                            OnTouchUpHandler(hWnd, ti[i]);
                        }
                    }
                }
                CloseTouchInputHandle((HTOUCHINPUT)lParam);
                delete [] ti;
            }
            break;

        case WM_DESTROY:
            // Clean up of application data: unregister window for multi-touch.
            if (!UnregisterTouchWindow(hWnd))
            {
                MessageBox(NULL, L"Cannot unregister application window for touch input", L"Error", MB_OK);
            }
            ASSERT(!IsTouchWindow(hWnd, NULL));
            // Destroy all the strokes
            {
                int i;
                for (i = 0; i < g_StrkColDrawing.Count(); ++i)
                {
                    delete g_StrkColDrawing[i];
                }
                for (i = 0; i < g_StrkColFinished.Count(); ++i)
                {
                    delete g_StrkColFinished[i];
                }
            }
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}





#pragma endregion