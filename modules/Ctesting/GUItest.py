import customtkinter as ctk
from tkinter import messagebox

quiz_data = [
    {
        "question": "Which of the following best describes the significance of 'Carlill v Carbolic Smoke Ball Co' in Australian contract law?",
        "choices": [
            "It established the modern doctrine of promissory estoppel",
            "It held that advertisements are always invitations to treat",
            "It confirmed that unilateral contracts can be formed by performance",
            "It denied the enforceability of vague contractual promises"
        ],
        "answer": "It confirmed that unilateral contracts can be formed by performance"
    },
    {
        "question": "According to the High Court in 'Ermogenous v Greek Orthodox Community of SA Inc', what is the correct approach to determining intention to create legal relations?",
        "choices": [
            "Assessing the subjective intention of the parties",
            "Inferring intention solely from the language used",
            "Looking at the objective circumstances of the agreement",
            "Determining whether consideration is present"
        ],
        "answer": "Looking at the objective circumstances of the agreement"
    },
    {
        "question": "Which principle was central to the reasoning in 'Waltons Stores (Interstate) Ltd v Maher'?",
        "choices": [
            "The postal rule",
            "Incorporation by reference",
            "Promissory estoppel",
            "Frustration of contract"
        ],
        "answer": "Promissory estoppel"
    },
    {
        "question": "In 'Masters v Cameron', which type of agreement was held not to create immediate legal obligations?",
        "choices": [
            "Where parties agree on all terms and intend immediate binding effect",
            "Where performance has begun before formalization",
            "Where parties agree but subject to the execution of a formal contract",
            "Where a deed is executed without consideration"
        ],
        "answer": "Where parties agree but subject to the execution of a formal contract"
    },
    {
        "question": "Which of the following is NOT a requirement for promissory estoppel as established in 'Waltons Stores v Maher'?",
        "choices": [
            "A clear and unequivocal promise or representation",
            "Reasonable reliance by the promisee",
            "A written agreement between the parties",
            "Detriment suffered due to reliance"
        ],
        "answer": "A written agreement between the parties"
    },
    {
        "question": "In the context of acceptance, what is the effect of the 'postal rule'?",
        "choices": [
            "Acceptance must be communicated face-to-face",
            "Acceptance is effective upon posting if reasonably contemplated",
            "Offers sent by post are not binding",
            "Revocation of offer is effective even after acceptance is posted"
        ],
        "answer": "Acceptance is effective upon posting if reasonably contemplated"
    },
    {
        "question": "In 'Placer Development Ltd v Commonwealth', why was the purported promise unenforceable?",
        "choices": [
            "It lacked consideration",
            "It was too vague and uncertain",
            "It was based on an illegal subject matter",
            "It was a social agreement"
        ],
        "answer": "It was too vague and uncertain"
    },
    {
        "question": "What is the general rule regarding silence and acceptance in Australian contract law?",
        "choices": [
            "Silence always indicates acceptance",
            "Silence cannot amount to acceptance unless expressly agreed",
            "Silence is valid if the offeree previously accepted terms silently",
            "Silence can bind a party if they receive benefit"
        ],
        "answer": "Silence cannot amount to acceptance unless expressly agreed"
    },
    {
        "question": "In 'Mobil Oil Australia Ltd v Lyndel Nominees Pty Ltd', what did the court hold regarding estoppel?",
        "choices": [
            "Estoppel is unavailable in commercial negotiations",
            "Estoppel can override statutory obligations",
            "There must be unconscionable conduct by the promisor",
            "Estoppel can operate without reliance"
        ],
        "answer": "There must be unconscionable conduct by the promisor"
    },
    {
        "question": "What distinguishes a 'mere puff' from a legally binding offer?",
        "choices": [
            "Whether the statement includes a promise of future intention",
            "Whether it is made in a domestic versus commercial context",
            "Whether a reasonable person would consider it a serious promise",
            "Whether it is communicated by letter or advertisement"
        ],
        "answer": "Whether a reasonable person would consider it a serious promise"
    }
]
# Function to display the current question and choices
def show_question():
    # Get the current question from the quiz_data list
    question = quiz_data[current_question]
    qs_label.configure(text=question["question"])

    # Display the choices on the buttons
    choices = question["choices"]
    for i in range(4):
        choice_btns[i].configure(text=choices[i], state="normal") # Reset button state

    # Clear the feedback label and disable the next button
    feedback_label.configure(text="")
    next_btn.configure(state="disabled")

# Function to check the selected answer and provide feedback
def check_answer(choice):
    # Get the current question from the quiz_data list
    question = quiz_data[current_question]
    selected_choice = choice_btns[choice].cget("text")

    # Check if the selected choice matches the correct answer
    if selected_choice == question["answer"]:
        # Update the score and display it
        global score
        score += 1
        score_label.configure(text="Score: {}/{}".format(score, len(quiz_data)))
        feedback_label.configure(text="Correct!", text_color="green")
    else:
        feedback_label.configure(
            text=f"Incorrect! The correct answer was: {question['answer']}",
            text_color="red"
        )
    
    # Disable all choice buttons
    for button in choice_btns:
        button.configure(state="disabled")

    # Wait 1.5 seconds before moving to the next question to allow users to see the feedback
    root.after(1500, next_question)

# Function to move to the next question
def next_question():
    global current_question
    current_question +=1

    if current_question < len(quiz_data):
        # If there are more questions, show the next question
        show_question()
    else:
        # Display final score
        qs_label.configure(text=f"🎉 Final Score: {score}/{len(quiz_data)} 🎉", font=("Helvetica Neue", 32, "bold"))
        qs_label.place(relx=0.5, rely=0.3, anchor="center")
        for button in choice_btns:
            button.pack_forget()
        feedback_label.configure(text="", fg_color=container._fg_color)
        next_btn.pack_forget()
        score_label.pack_forget()

        # Simple dance animation
        colors = ["#ff9999", "#99ff99", "#9999ff", "#ffff99", "#ffccff"]
        def dance_animation(index=0, count=0):
            if count < 10:
                container.configure(fg_color=colors[index % len(colors)])
                root.after(200, lambda: dance_animation(index + 1, count + 1))
            else:
                container.configure(fg_color="#FFF8DC")  # Reset to original

        dance_animation()

        def bounce_and_spin(count=0):
            if count < 20:
                dx = 10 * ((-1) ** count)
                dy = 5 * ((-1) ** (count // 2))
                angle = (count * 18) % 360
                qs_label.place_configure(relx=0.5 + dx / 900, rely=0.3 + dy / 700)
                qs_label.configure(text=f"🎉 Final Score: {score}/{len(quiz_data)} 🎉")
                root.after(100, lambda: bounce_and_spin(count + 1))
            else:
                qs_label.place_configure(relx=0.5, rely=0.3)
                qs_label.configure(text=f"🎉 Final Score: {score}/{len(quiz_data)} 🎉")

        bounce_and_spin()

ctk.set_appearance_mode("light")
ctk.set_default_color_theme("blue")
root = ctk.CTk()
root.title("Greek Mythology Master Quiz")
root.geometry("900x700")
root.configure(fg_color="#fefefe")

# Add a padding frame to center and separate elements visually
container = ctk.CTkFrame(root, fg_color="#FFF8DC")
container.pack(padx=20, pady=20, fill="both", expand=True)


# Heading label at the top of the quiz
heading_label = ctk.CTkLabel(
    container,
    text="⚖️ USYD LAWS1015 Contracts Quiz 📚",
    anchor="center",
    font=("Helvetica Neue", 42, "bold"),
    corner_radius=10,
    fg_color="#f5f5f5",
    text_color="#1c1c1c"
)
heading_label.pack(pady=20)

# Create the question label
qs_label = ctk.CTkLabel(
    container,
    anchor="center",
    wraplength=700,
    justify="center",
    font=("Helvetica Neue", 28, "bold"),
    corner_radius=10,
    fg_color="#ffffff",
    text_color="#222222"
)
qs_label.place(relx=0.5, rely=0.3, anchor="center")
# qs_label.pack(pady=20)
# qs_label.place(relx=0.5, rely=0.4, anchor="center")

# Create the choice buttons
choice_btns = []
for i in range(4):
    button = ctk.CTkButton(
        container,
        command=lambda i=i: check_answer(i),
        font=("Helvetica Neue", 22, "bold"),
        height=60,
        corner_radius=15,
        fg_color="#4a90e2",
        text_color="#ffffff",
        hover_color="#6495ED"
    )
    button.pack(pady=8, padx=40, fill="x")
    choice_btns.append(button)

# Create the feedback label
feedback_label = ctk.CTkLabel(
    container,
    anchor="center",
    font=("Helvetica Neue", 22),
    corner_radius=10,
    fg_color="#f5f5f5",
    text_color="#1a1a1a"
)
feedback_label.pack(pady=30)

# Initialize the score
score = 0

# Create the score label
score_label = ctk.CTkLabel(
    container,
    text="Score: 0/{}".format(len(quiz_data)),
    anchor="center",
    font=("Helvetica Neue", 22),
    corner_radius=10,
    fg_color="#f5f5f5",
    text_color="#1a1a1a"
)
score_label.pack(pady=20)

# Create the next button
next_btn = ctk.CTkButton(
    container,
    text="Next",
    command=next_question,
    state="disabled",
    font=("Helvetica Neue", 22, "bold"),
    height=60,
    corner_radius=15,
    fg_color="#4a90e2",
    text_color="#ffffff",
    hover_color="#FF4500"
)
next_btn.pack(pady=20)

# Initialize the current question index
current_question = 0

# Show the first question
show_question()

# Start the main event loop
root.mainloop()