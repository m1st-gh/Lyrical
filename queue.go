package main

import (
	"fmt"
)

type Queue[T any] struct {
	contents []T
	index    int
	size     int
}

func NewQueue[T any]() *Queue[T] {
	return &Queue[T]{
		contents: make([]T, 0, 64),
		index:    0,
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
	if q.index+1 >= q.size {
		return zero, fmt.Errorf("end of queue")
	}
	q.index++
	return q.contents[q.index], nil
}

func (q *Queue[T]) Prev() (T, error) {
	var zero T
	if q.index <= 0 {
		return zero, fmt.Errorf("start of queue")
	}
	q.index--
	return q.contents[q.index], nil
}

func (q *Queue[T]) Dequeue(index int) (T, error) {
	var zero T
	if index >= q.size {
		return zero, fmt.Errorf("index out of range: %d (size: %d)", index, q.size)
	}
	item := q.contents[index]
	copy(q.contents[index:], q.contents[index+1:])
	q.contents = q.contents[:q.size-1]
	q.size--
	if q.index > 0 {
		q.index--
	}
	return item, nil
}
func (q *Queue[T]) Items() []T {
	return q.contents
}

func (q *Queue[T]) Empty() bool {
	return q.size == 0
}

func (q *Queue[T]) Size() int {
	return q.size
}

func (q *Queue[T]) Index() int {
	return q.index
}

func (q *Queue[T]) Clear() {
	q.contents = make([]T, 0, 32)
	q.index = 0
	q.size = 0
}
