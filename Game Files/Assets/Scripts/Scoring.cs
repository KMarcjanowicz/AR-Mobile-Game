using System.Collections;
using System.Collections.Generic;
using Unity.VisualScripting;
using UnityEngine;
using UnityEngine.UI;
using TMPro;

public class Scoring : MonoBehaviour
{
    public static int Score;

    public static void SetScoreText(GameObject _TextObj)
    {
        Debug.Log(_TextObj);
        _TextObj.GetComponent<TMP_Text>().text = "" + Score;
    }

    public static void SetScoreToZero()
    {
        Score = 0;
    }
}
