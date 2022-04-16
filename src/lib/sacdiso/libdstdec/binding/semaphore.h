/*
* SACD Decoder plugin
* Copyright (c) 2011-2020 Maxim V.Anisiutkin <maxim.anisiutkin@gmail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with FFmpeg; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef _SEMAPHORE_H_INCLUDED
#define _SEMAPHORE_H_INCLUDED

#include <mutex>
#include <condition_variable>

using std::mutex;
using std::condition_variable;
using std::lock_guard;
using std::unique_lock;

class semaphore {
	mutex m_mtx;
	condition_variable m_cv;
	int m_cnt = 0;
public:
	semaphore() = default;
	semaphore(const semaphore& sem) = delete;
	semaphore(semaphore&& sem) = delete;
	semaphore operator=(const semaphore& sem) = delete;
	void notify() {
		lock_guard<mutex> lock(m_mtx);
		m_cnt++;
		m_cv.notify_one();
	}
	void wait() {
		unique_lock<mutex> lock(m_mtx);
		while (!m_cnt) {
			m_cv.wait(lock);
		}
		m_cnt--;
	}
	bool try_wait() {
		lock_guard<mutex> lock(m_mtx);
		if (m_cnt) {
			m_cnt--;
			return true;
		}
		return false;
	}
};

#endif
