// Copyright (C) 2024 mykaa. All rights reserved.

#pragma once

/**
 * Tween Manager
 */
template <class T>
class NSTWEEN_API NsTweenManager
{
    typedef typename TDoubleLinkedList<T*>::TDoubleLinkedListNode TNode;

// Variables
private:

    /** Active Tweens */
    TDoubleLinkedList<T*>* ActiveTweens;

    /** Recycled Tweens */
    TDoubleLinkedList<T*>* RecycledTweens;

    /** Tweens to activate on the next update */
    TDoubleLinkedList<T*>* TweensToActivate;

// Functions
public:

    /** Constructor */
    NsTweenManager(int Capacity)
    {
        ActiveTweens = new TDoubleLinkedList<T*>();
        RecycledTweens = new TDoubleLinkedList<T*>();
        TweensToActivate = new TDoubleLinkedList<T*>();
        for (int i = 0; i < Capacity; ++i)
        {
            RecycledTweens->AddTail(new T());
        }
    }

    /** Desstructor */
    ~NsTweenManager()
    {
        TNode* CurNode = RecycledTweens->GetHead();
        while (CurNode != nullptr)
        {
            delete CurNode->GetValue();
            CurNode = CurNode->GetNextNode();
        }
        delete RecycledTweens;

        CurNode = ActiveTweens->GetHead();
        while (CurNode != nullptr)
        {
            delete CurNode->GetValue();
            CurNode = CurNode->GetNextNode();
        }
        delete ActiveTweens;

        CurNode = TweensToActivate->GetHead();
        while (CurNode != nullptr)
        {
            delete CurNode->GetValue();
            CurNode = CurNode->GetNextNode();
        }
        delete TweensToActivate;
    }

    /** Ensure capacity */
    void EnsureCapacity(int Num)
    {
        int NumExistingTweens = ActiveTweens->Num() + TweensToActivate->Num() + RecycledTweens->Num();
        for (int i = NumExistingTweens; i < Num; ++i)
        {
            RecycledTweens->AddTail(new T());
        }
    }

    /** Get current capacity */
    int GetCurrentCapacity()
    {
        return ActiveTweens->Num() + TweensToActivate->Num() + RecycledTweens->Num();
    }

    /** Update */
    void Update(float UnscaledDeltaSeconds, float DilatedDeltaSeconds, bool bIsGamePaused)
    {
        // add pending tweens
        TNode* CurNode = TweensToActivate->GetHead();
        while (CurNode != nullptr)
        {
            TNode* NodeToActivate = CurNode;
            CurNode = CurNode->GetNextNode();
            NodeToActivate->GetValue()->Start();
            TweensToActivate->RemoveNode(NodeToActivate, false);
            ActiveTweens->AddTail(NodeToActivate);
        }

        // update tweens
        CurNode = ActiveTweens->GetHead();
        while (CurNode != nullptr)
        {
            NsTweenInstance* CurTween = static_cast<NsTweenInstance*>(CurNode->GetValue());
            CurTween->Update(UnscaledDeltaSeconds, DilatedDeltaSeconds, bIsGamePaused);
            TNode* NextNode = CurNode->GetNextNode();
            if (!CurTween->bIsActive)
            {
                ActiveTweens->RemoveNode(CurNode, false);
                RecycleTween(CurNode);
            }
            CurNode = NextNode;
        }
    }

    /** Clear active tweens */
    void ClearActiveTweens()
    {
        TNode* CurNode = TweensToActivate->GetHead();
        while (CurNode != nullptr)
        {
            TNode* NodeToRecycle = CurNode;
            CurNode = CurNode->GetNextNode();

            NodeToRecycle->GetValue()->Destroy();
            TweensToActivate->RemoveNode(NodeToRecycle, false);
            RecycledTweens->AddTail(NodeToRecycle);
        }

        CurNode = ActiveTweens->GetHead();
        while (CurNode != nullptr)
        {
            TNode* NodeToRecycle = CurNode;
            CurNode = CurNode->GetNextNode();
            ActiveTweens->RemoveNode(NodeToRecycle, false);
            RecycledTweens->AddTail(NodeToRecycle);
        }
    }

    /** Create Tween */
    T* CreateTween()
    {
        TNode* NewTween = GetNewTween();
        TweensToActivate->AddTail(NewTween);
        return NewTween->GetValue();
    }

private:

    /** Get new tween */
    TNode* GetNewTween()
    {
        TNode* Node = RecycledTweens->GetHead();
        if (Node != nullptr)
        {
            RecycledTweens->RemoveNode(Node, false);
            return Node;
        }
        return new TNode(new T());
    }

    /** Recycle tween */
    void RecycleTween(TNode* ToRecycle)
    {
        RecycledTweens->AddTail(ToRecycle);
    }
};
