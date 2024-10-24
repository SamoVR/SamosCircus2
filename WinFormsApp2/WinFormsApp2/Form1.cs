using System;
using System.Data; // This is required to use DataTable for expression evaluation
using System.Windows.Forms;

namespace WinFormsApp2
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void Divide()
        {
            richTextBox1.Text = richTextBox1.Text + "/";
        }

        private void Times()
        {
            richTextBox1.Text = richTextBox1.Text + "*";
        }

        private void Minus()
        {
            richTextBox1.Text = richTextBox1.Text + "-";
        }

        private void Plus()
        {
            richTextBox1.Text = richTextBox1.Text + "+";
        }

        private void Result()
        {
            try
            {
                string expression = richTextBox1.Text.Replace(',', '.');

                DataTable dt = new DataTable();
                var result = dt.Compute(expression, string.Empty);

                label1.Text = richTextBox1.Text + "=";
                richTextBox1.Text = result.ToString();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Invalid expression");
            }
        }


        private void AddNumbers(long number)
        {
            if (richTextBox1.Text == "0")
            {
                richTextBox1.Text = number.ToString();
            }
            else
            {
                richTextBox1.Text = richTextBox1.Text + number;
            }
        }

        private void Backspace()
        {
            if (richTextBox1.Text.Length > 0)
            {
                richTextBox1.Text = richTextBox1.Text.Substring(0, richTextBox1.Text.Length - 1);
            }

            if (string.IsNullOrEmpty(richTextBox1.Text))
            {
                richTextBox1.Text = "0";
            }
            label1.Text = " ";
        }

        private void button9_Click(object sender, EventArgs e)
        {
            //1
            AddNumbers(1);
        }

        private void button8_Click(object sender, EventArgs e)
        {
            //2
            AddNumbers(2);
        }

        private void button7_Click(object sender, EventArgs e)
        {
            //3
            AddNumbers(3);
        }

        private void button6_Click(object sender, EventArgs e)
        {
            //4
            AddNumbers(4);
        }

        private void button5_Click(object sender, EventArgs e)
        {
            //5
            AddNumbers(5);
        }

        private void button4_Click(object sender, EventArgs e)
        {
            //6
            AddNumbers(6);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            //7
            AddNumbers(7);
        }

        private void button2_Click(object sender, EventArgs e)
        {
            //8
            AddNumbers(8);
        }

        private void button3_Click(object sender, EventArgs e)
        {
            //9
            AddNumbers(9);
        }

        private void richTextBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void button14_Click(object sender, EventArgs e)
        {
            // backspace
            Backspace();
        }

        private void button10_Click(object sender, EventArgs e)
        {
            //plus
            Plus();
        }

        private void button13_Click(object sender, EventArgs e)
        {
            //Divide
            Divide();
        }

        private void button11_Click(object sender, EventArgs e)
        {
            //Minus
            Minus();
        }

        private void button12_Click(object sender, EventArgs e)
        {
            //Times
            Times();
        }

        private void button15_Click(object sender, EventArgs e)
        {
            //0
            AddNumbers(0);
        }

        private void button16_Click(object sender, EventArgs e)
        {
            //Result
            Result();
        }
    }
}
