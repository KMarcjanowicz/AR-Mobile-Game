using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;
using UnityEngine.SceneManagement;

public class OpenCVFaceDetection : MonoBehaviour
{
    public static List<Vector2> NormalizedFacePositions { get; private set; }
    public static Vector2 CameraResolution;
    public static bool isEyeOpen;
    [SerializeField] public GameObject sendTo;

    /// <summary>
    /// Downscale factor to speed up detection.
    /// </summary>
    private const int DetectionDownScale = 1;

    private bool _ready;
    private int _maxFaceDetectCount = 1;
    private CvCircle[] _faces;

    void Start()
    {
        int camWidth = 0, camHeight = 0;
        int result = OpenCVInterop.Init(ref camWidth, ref camHeight);
        if (result < 0)
        {
            if (result == -1)
            {
                Debug.LogWarningFormat("[{0}] Failed to find cascades definition.", GetType());
            }
            else if (result == -2)
            {
                Debug.LogWarningFormat("[{0}] Failed to open camera stream.", GetType());
            }

            return;
        }

        CameraResolution = new Vector2(camWidth, camHeight);
        _faces = new CvCircle[_maxFaceDetectCount];
        NormalizedFacePositions = new List<Vector2>();
        OpenCVInterop.SetScale(DetectionDownScale);
        _ready = true;
    }

    void OnApplicationQuit()
    {
        if (_ready)
        {
            OpenCVInterop.Close();
        }
    }

    void Update()
    {
        if (!_ready)
            return;

        int detectedFaceCount = 0;
        unsafe
        {
            fixed (CvCircle* outFaces = _faces)
            {
                OpenCVInterop.Detect(outFaces, _maxFaceDetectCount, ref detectedFaceCount);
            }
        }

        NormalizedFacePositions.Clear();
        for (int i = 0; i < detectedFaceCount; i++)
        {
            NormalizedFacePositions.Add(new Vector2((_faces[i].X * DetectionDownScale) / CameraResolution.x, 1f - ((_faces[i].Y * DetectionDownScale) / CameraResolution.y)));
            
            if(sendTo != null)
            {
                string sceneName = SceneManager.GetActiveScene().name;
                if(sceneName == "MainMenu")
                {
                    ManageScenes sendScript = sendTo.GetComponent<ManageScenes>();
                    if (sendScript != null)
                    {
                        sendScript.facePos.x = NormalizedFacePositions[i].x;
                        sendScript.facePos.y = NormalizedFacePositions[i].y;
                    }
                }
                else if (sceneName != "MainMenu")
                {
                    MovePlayer sendScript = sendTo.GetComponent<MovePlayer>();
                    if (sendScript != null)
                    {
                        sendScript.isEyeOpen = _faces[i].isOpen;
                    }
                }
                
            }
        }
    }
}

// Define the functions which can be called from the .dll.
internal static class OpenCVInterop
{
    [DllImport("UnityOpenCVlink")]
    internal static extern int Init(ref int outCameraWidth, ref int outCameraHeight);

    [DllImport("UnityOpenCVlink")]
    internal static extern int Close();

    [DllImport("UnityOpenCVlink")]
    internal static extern int SetScale(int downscale);

    [DllImport("UnityOpenCVlink")]
    internal unsafe static extern void Detect(CvCircle* outFaces, int maxOutFacesCount, ref int outDetectedFacesCount);
}

// Define the structure to be sequential and with the correct byte size (3 ints = 4 bytes * 3 = 12 bytes + 2 bytes from bool)
[StructLayout(LayoutKind.Sequential, Size = 14)]
public struct CvCircle
{
    public int X, Y, Radius;
    public bool isOpen;
}