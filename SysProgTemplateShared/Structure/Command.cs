﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.ComponentModel.DataAnnotations;
using System.ComponentModel;
using SysProgTemplateShared.Exceptions;
using SysProgTemplateShared.Dto;
using System.Text.RegularExpressions;


namespace SysProgTemplateShared.Structure
{
    public class Command 
    {
        [Required]
        [StringLength(50, MinimumLength = 1)]
        public string Name { get; set; } = default!;
        
        [Required]
        [Range(0,255)]
        public int Code { get; set; } = default!;

        [Required]
        [Range(1, 255)] 
        public int Length { get; set; } = default!; 

        public Command() {}

        public Command(CommandDto dto)
        {
            string command = $"{dto.Name} {dto.Code} {dto.Length}";

            // Name 
            if (!"qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM".Contains(dto.Name[0])) throw new AssemblerException($"Название команды должно начинатья с латинской буквы: {command}");

            if (!dto.Name.All(c => "1234567890qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM".Contains(c))) throw new AssemblerException($"Название команды должно состоять из латинских букв и цифр: {command}");

            if (dto.Name.Length <= 0)
                throw new AssemblerException($"Название команды должно содержать как минимум один символ: {command} "); 

            Name = dto.Name;


            // Code 
            int code;
            try { code = Convert.ToInt32(dto.Code, 16); } 
            catch { throw new AssemblerException($"Код команды должен быть целым числом в 16-ричном формате:  {command}"); } 

            if (code < 0 || code > 255)
                throw new AssemblerException($"Код команды должен быть значением от 0 до 255:  {command}");

            Code = code;


            // Length 
            int length;
            try { length = Convert.ToInt32(dto.Length, 16); }
            catch { throw new AssemblerException($"Код команды должен быть целым числом в 16-ричном формате:  {command}"); }

            if (length < 1 || length > 4 || length == 3)
                throw new AssemblerException($"Длина команды должна быть 1,2 или 4:  {command}");

            Length = length; 
        }
    }
}
