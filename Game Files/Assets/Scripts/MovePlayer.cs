using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using UnityEditor;
using UnityEngine;
using UnityEngine.SceneManagement;

public class MovePlayer : MonoBehaviour
{
    [SerializeField] GameObject ScoreField;
    //array of positions
    [SerializeField] Transform[] Positions;
    Transform NextPos;
    [SerializeField] Vector3 InitialPosition;
    Vector3 currentPosition;
    

    [SerializeField] float ObjectSpeed = 1.0f;
    private const float SPEED = 3.0f;
    int NextPosIndex = 0;
    
   
    public bool isEyeOpen = true;
    private bool playerAttempt = false;
    private String SceneName;

    // Start is called before the first frame update
    void Start()
    {
        NextPos = Positions[0];
        SceneName = gameObject.scene.name;
        Scoring.SetScoreText(ScoreField);
    }

    // Update is called once per frame
    void Update()
    {
        HandleTestMove();
        moveGameObject();
        tryToFitIt();
        checkIfEyeOpen();
        checkIfWinner();
    }

    //Called when interacting with trigger field
    private void OnTriggerEnter2D(Collider2D collision)
    {
        if(collision.gameObject.tag == "Fail")
        {
            transform.position = new Vector3(currentPosition.x, -3.5f, 0);
            playerAttempt = false;
        }
    }

    private void moveGameObject()
    {
        if(!playerAttempt)
        {
            if (transform.position == NextPos.position)
            {
                NextPosIndex++;
                if (NextPosIndex >= Positions.Length)
                {
                    NextPosIndex = 0;
                }
                NextPos = Positions[NextPosIndex];
            }
            else
            {
                transform.position = Vector3.MoveTowards(transform.position, NextPos.position, ObjectSpeed * Time.deltaTime);
            }
        }
    }
    private void checkIfEyeOpen()
    {
        if(!isEyeOpen && !playerAttempt)
        {
            playerAttempt = true;
            Vector3 currentPosition = transform.position;
            Scoring.Score += 1;
            Scoring.SetScoreText(ScoreField);
        }
    }

    private void tryToFitIt()
    {
        if(Input.GetKey(KeyCode.Space) && !playerAttempt){  // test only, later will be received data from eye closing
            playerAttempt = true;                           // if eye blink = player attempts to fit the item
        }

        if(playerAttempt)
        {
            currentPosition = transform.position;
            float moveY = 1.0f;
            Vector3 moveDir = new Vector3(0.0f, moveY, 0.0f).normalized;
            transform.position += moveDir * SPEED * Time.deltaTime;
        }
    }

    private void checkIfWinner()
    {
        if (SceneName == "SampleScene1")
        {
            if (transform.position.y >= 1.0f)
            {
                transform.position = InitialPosition;
                playerAttempt = false;
                SceneManager.LoadScene("Scenes/SampleScene2");
            }
        }
        else if (SceneName == "SampleScene2")
        {
            if (transform.position.y >= 4.0f)
            {
                transform.position = InitialPosition;
                playerAttempt = false;
                Scoring.SetScoreToZero();
                SceneManager.LoadScene("Scenes/MainMenu");
            }
                
        }      
    }

    
    private void HandleTestMove()
    {
        float moveX = 0.0f;
        float moveY = 0.0f;

        if(Input.GetKey(KeyCode.W) || Input.GetKey(KeyCode.UpArrow))
        {
            moveY= +1.0f;
        }
        if (Input.GetKey(KeyCode.S) || Input.GetKey(KeyCode.DownArrow))
        {
            moveY = -1.0f;
        }
        if (Input.GetKey(KeyCode.A) || Input.GetKey(KeyCode.LeftArrow))
        {
            moveX = -1.0f;
        }
        if (Input.GetKey(KeyCode.D) || Input.GetKey(KeyCode.RightArrow))
        {
            moveX = +1.0f;
        }
        if (Input.GetKey(KeyCode.Space))
        {
            tryToFitIt();
        }

        Vector3 moveDir = new Vector3(moveX, moveY, 0.0f).normalized;

        transform.position += moveDir * SPEED * Time.deltaTime;
    }
    
}
