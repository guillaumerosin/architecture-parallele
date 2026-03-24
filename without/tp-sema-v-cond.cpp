#include <semaphore>
#include <atomic>

#define Nwriters 3
#define Nreaders 2
#define N 10
int buffer[N];

std::mutex mutex_writer;  //
int writing_indice = 0;
std::counting_semaphore<N> sem_write(N);

std::mutex mutex_reader;
int reading_indice = 0;
std::counting_semaphore<0> sem_read(0);

std::mutex screen_mutex;
std::atomic<int> job2do = 0;

bool end = false;

void writer(int id)
{
    for (int i = 0; i < N * 2; ++i)
    {

        sem_write.try_acquire();

        mutex_writer.lock();
        int pos = writing_indice;
        writing_indice++;
        if (writing_indice == N) writing_indice = 0;
        mutex_writer.unlock();

        int val = rand() % 64;
        buffer[pos] = val;
        job2do.fetch_add(1);

        screen_mutex.lock();
        std::cout << "Writer " << id << " : Buffer[" << pos << "]=" << val << std::endl;
        screen_mutex.unlock();

        sem_read.release();
    }
}

void reader(int id)
{
    while (1)
    {
        if (sem_read.try_acquire())
        {
            mutex_reader.lock();
            int pos = reading_indice;
            reading_indice++;
            if (reading_indice == N) reading_indice = 0;
            mutex_reader.unlock();
            job2do.fetch_sub(1);
            int val = buffer[pos];

            screen_mutex.lock();
            std::cout << "Reader " << id << " : Buffer[" << pos << "]=" << val << std::endl;
            screen_mutex.unlock();

            sem_write.release();
        }
        else
        {
            if (end)
            {
                // suite non visible sur les images
            }
        }
    }
}

int main()
{
    std::thread* writers[Nwriters];
    for (int i = 0; i < Nwriters; ++i) writers[i] = new std::thread(writer, i + 1);

    std::thread* readers[Nreaders];
    for (int i = 0; i < Nreaders; ++i) readers[i] = new std::thread(reader, 11 + i);

    for (int i = 0; i < Nwriters; ++i) writers[i]->join();

    end = true;

    for (int i = 0; i < Nreaders; ++i) readers[i]->join();

    for (int i = 0; i < Nwriters; ++i) delete writers[i];
    for (int i = 0; i < Nreaders; ++i) delete readers[i];
}