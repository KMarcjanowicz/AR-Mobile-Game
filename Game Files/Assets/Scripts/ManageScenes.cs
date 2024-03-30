using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

public class ManageScenes : MonoBehaviour
{
    public Vector2 facePos;
    private bool faceLeft;
    private bool faceRight;
    private bool howTo;
    private float scoreLeft;
    private float scoreRight;
    private string sceneName;

    GameObject MainMenuCanvas;
    GameObject HowToCanvas;

    private void Start()
    {
        howTo = false;
        faceLeft = false;
        faceRight = false;
        scoreLeft = 0.0f;
        scoreRight = 0.0f;
        sceneName = gameObject.scene.name;

        MainMenuCanvas = GameObject.Find("MainMenu");
        HowToCanvas = GameObject.Find("HowTo");
        Debug.Log(HowToCanvas);
        HowToCanvas.SetActive(false);
    }
    void LateUpdate()
    {

        handleFaceInput();
    }
    public void ChangeScene(string SceneName)
    {
        SceneManager.LoadScene("Scenes/" + SceneName);
    }

    public void handleFaceInput()
    {
        if (facePos.x == 0.0f && facePos.y == 0.0f)
            return;

        if(facePos.x >= 0.55f)
        {
            faceLeft = true;
            faceRight = false;

            scoreLeft += 1.0f * Time.deltaTime;
            if (scoreRight != 0.0f) { scoreRight = 0.0f; };
        }
        if(facePos.x <= 0.10f)
        {
            faceRight = true;
            faceLeft = false;

            scoreRight += 1.0f * Time.deltaTime;
            if (scoreLeft != 0.0f) { scoreLeft = 0.0f; };
        }
        if(!howTo)
        {
            if (faceRight) { if (scoreRight >= 4.0f) { changeCanvas(); scoreRight = 0; }; };
            if (faceLeft) { if (scoreLeft >= 4.0f) { ChangeScene("SampleScene1"); }; };
        }
        else if(howTo)
        {
            if (faceRight) { if (scoreRight >= 4.0f) { changeCanvas(); scoreRight = 0;  }; };
        }
        
    }

    public void changeCanvas() {
        if (howTo)
        {
            howTo = false;
            HowToCanvas.SetActive(false);
            MainMenuCanvas.SetActive(true);
        }
        else
        {
            howTo = true;
            HowToCanvas.SetActive(true);
            MainMenuCanvas.SetActive(false);
        }
    }


}
