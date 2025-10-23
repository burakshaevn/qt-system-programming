using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using SysProgTemplate.Components;
using SysProgTemplateShared.Structure;
using SysProgTemplateShared;
using System.Text.Json;
using SysProgTemplateShared.Exceptions;
using SysProgTemplateShared.Dto;
using SysProgTemplateShared.Helpers;
using static SysProgTemplate.Samples; 

namespace SysProgTemplate
{
    /// <summary>
    /// Логика взаимодействия для MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private Assembler Assembler {get; set; } = new Assembler();

        // Исходный код
        private string SourceCode { get; set; } = Samples.StraightSample;

        private string AddressingMode { get; set; } = "Straight"; 
        
        private TextBox SourceCodeTextBox { get; set; }

        // Таблица кодов операций 
        //private List<CommandDto> AvailibleCommandsDto { get; set; } = default!; 
        private TextBox CommandsTextBox { get; set; } 
 
        // Вспомогатеьная таблица 
        private TextBox FirstPassTextBox { get; set; }

        // ТСИ 
        private TextBox TSITextBox { get; set; }

        // ТН
        private TextBox TNTextBox { get; set; }

        //Двоичный код 
        private TextBox SecondPassTextBox { get; set; } 

        // Ошибки первого прохода 
        private TextBox FirstPassErrorsTextBox { get; set; }

        // Ошибки второго прохода 
        private TextBox SecondPassErrorsTextBox { get; set; }

        // кнопки 
        private Button FirstPassButton { get; set; }
        private Button SecondPassButton { get; set; }

        private ComboBox ModeComboBox { get; set; } 
        
        public MainWindow()
        {
            InitializeComponent();

            SourceCodeTextBox = this.SourceCode_TextBox;
            SourceCodeTextBox.Text = SourceCode;

            CommandsTextBox = this.Commands_TextBox;
            CommandsTextBox.Text = string.Join("\n", Assembler.AvailibleCommands.Select(c => $"{c.Name} {c.Code} {c.Length}")); 

            FirstPassTextBox = this.FirstPass_TextBox;
            SecondPassTextBox = this.SecondPass_TextBox; 

            TSITextBox = this.TSI_TextBox;
            TNTextBox = this.TN_TextBox;

            FirstPassErrorsTextBox = this.FirstPassErrors_TextBox; 
            SecondPassErrorsTextBox = this.SecondPassErrors_TextBox;

            FirstPassButton = this.FirstPass_Button; 
            SecondPassButton = this.SecondPass_Button;

            ModeComboBox = this.Mode_ComboBox;
            ModeComboBox.SelectedIndex = 0; 
        }

        private void FirstPass_ButtonClick(object sender, RoutedEventArgs e)
        {
           SecondPassButton.IsEnabled = true; 

            try
            {
                TSITextBox.Text = null; 
                TNTextBox.Text = null; 
                FirstPassTextBox.Text = null;
                SecondPassTextBox.Text = null; 
                FirstPassErrorsTextBox.Text = null;
                SecondPassErrorsTextBox.Text = null;

                var newCommands = Parser.TextToCommandDtos(CommandsTextBox.Text);
                Assembler.SetAvailibleCommands(newCommands);

                Assembler.ClearTSI();
                Assembler.ClearTN(); 

                var sourceCode = Parser.ParseCode(SourceCodeTextBox.Text); 
                FirstPassTextBox.Text = string.Join("\n", Assembler.FirstPass(sourceCode, AddressingMode));
                TSITextBox.Text = string.Join("\n", Assembler.TSI.Select(w => $"{w.Name} {w.Address.ToString("X6")}"));
            }
            catch (AssemblerException ex) 
            {
                FirstPassErrorsTextBox.Text = $"Ошибка: {ex.Message}"; 
            }

            if (!string.IsNullOrEmpty(FirstPassErrorsTextBox.Text))
            {
                SecondPassButton.IsEnabled = false; 
            }
        }

        private void SecondPass_ButtonClick(object sender, RoutedEventArgs e)
        {
            Assembler.ClearTN();
            TNTextBox.Text = null; 
            SecondPassTextBox.Text = null;
            SecondPassErrorsTextBox.Text = null;

            if (FirstPassTextBox.Text == String.Empty) return; 

            try
            {
                var firstPassCode = Parser.ParseCode(FirstPassTextBox.Text); 
                SecondPassTextBox.Text = string.Join("\n", Assembler.SecondPass(firstPassCode));
                TNTextBox.Text = string.Join("\n", Assembler.TN); 
            }
            catch(AssemblerException ex)
            {
                SecondPassErrorsTextBox.Text = $"Ошибка: {ex.Message}";

            }
        }

        private void Mode_ComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            switch ((ModeComboBox.SelectedItem as TextBlock).Name)
            {
                case "Straight":
                    SourceCode = Samples.StraightSample;
                    AddressingMode = "Straight"; 
                    SourceCodeTextBox.Text = SourceCode; 
                    break;

                case "Relative":
                    SourceCode = Samples.RelativeSample;
                    AddressingMode = "Relative";
                    SourceCodeTextBox.Text = SourceCode; 
                    break;

                case "Mixed":
                    SourceCode = Samples.MixedSample;
                    AddressingMode = "Mixed";
                    SourceCodeTextBox.Text = SourceCode;
                    break; 
            }
        }
    }
}
