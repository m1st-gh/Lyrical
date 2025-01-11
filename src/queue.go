package main

import (
	"fmt"
	"math/rand"
	"reflect"
)

type Queue[T any] struct {
	contents []T
	current  int
	size     int
}

func NewQueue[T any]() *Queue[T] {
	return &Queue[T]{
		contents: make([]T, 0, 64),
		current:  0,
		size:     0,
	}
}

func (q *Queue[T]) Peek(index int) (T, error) {
	var zero T
	if index >= q.size {
		return zero, fmt.Errorf("index out of range: %d (size: %d)", index, q.size)
	}
	return q.contents[index], nil
}

func (q *Queue[T]) Enqueue(items ...T) {
	q.contents = append(q.contents, items...)
	q.size += len(items)
}

func (q *Queue[T]) Next() (T, error) {
	var zero T
	if q.current+1 >= q.size {
		return zero, fmt.Errorf("end of queue")
	}
	q.current++
	return q.contents[q.current], nil
}

func (q *Queue[T]) Prev() (T, error) {
	var zero T
	if q.current <= 0 {
		return zero, fmt.Errorf("start of queue")
	}
	q.current--
	return q.contents[q.current], nil
}
func (q *Queue[T]) Dequeue() (T, error) {
	var zero T
	if q.size == 0 {
		return zero, fmt.Errorf("queue is empty")
	}
	item := q.contents[0]
	copy(q.contents[0:], q.contents[1:])
	q.contents = q.contents[:q.size-1]
	q.size--

	return item, nil
}

func (q *Queue[T]) Pop(i int) (T, error) {
	var zero T
	if i >= q.size {
		return zero, fmt.Errorf("index out of range: %d (size: %d)", q.current, q.size)
	}
	item := q.contents[i]
	copy(q.contents[i:], q.contents[i+1:])
	q.contents = q.contents[:q.size-1]
	q.size--
	if i <= q.current && q.current > 0 {
		q.current--
	}
	return item, nil
}

func (q *Queue[T]) Shuffle() error {
	current := q.contents[q.current]
	rand.Shuffle(q.size, func(i, j int) {
		q.contents[i], q.contents[j] = q.contents[j], q.contents[i]
	})
	for i, item := range q.contents {
		if reflect.DeepEqual(item, current) {
			q.Pop(i)
			q.contents = append([]T{current}, q.contents...)
			q.size++
			q.current = 0
		}
	}
	return nil
}

func (q *Queue[T]) Items() []T {
	return q.contents
}

func (q *Queue[T]) IsEmpty() bool {
	return q.size == 0
}

func (q *Queue[T]) Size() int {
	return q.size
}

func (q *Queue[T]) Current() int {
	return q.current
}

func (q *Queue[T]) Clear() {
	q.contents = make([]T, 0, 32)
	q.current = 0
	q.size = 0
}
