PAGE_SIZE = 1024

page_table = {
    0: 5,
    1: 2,
    2: 7,
    3: 0
}


def translate(addr):
    page = addr // PAGE_SIZE
    offset = addr % PAGE_SIZE

    print(f"Virtual address: {addr}")
    print(f"Page: {page}")
    print(f"Offset: {offset}")

    if page not in page_table:
        print("Page not present\n")
        return None

    frame = page_table[page]
    physical = frame * PAGE_SIZE + offset

    print(f"Frame: {frame}")
    print(f"Physical address: {physical}\n")

    return physical


with open("refs.txt") as file:
    for line in file:
        translate(int(line.strip()))
