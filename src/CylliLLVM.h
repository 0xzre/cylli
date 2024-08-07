/*
    Cylli to LLVM IR Compiler
*/

#ifndef CYLLI_LLVM_H
#define CYLLI_LLVM_H

#include <string>
#include <iostream>
#include <regex>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

#include "./parser/CylliParser.h"

using syntax::CylliParser;

class CylliLLVM
{
public:
    CylliLLVM() : parser(std::make_unique<CylliParser>())
    {
        moduleInit();
        setupExernalFunctions();
    }

    void exec(const std::string &cylliCode)
    {
        // 1. Parse the cylli program
        auto ast = parser->parse(cylliCode);

        // 2. Generate LLVM IR
        // auto ir = CylliLLVMGenerator::generate(ast);

        compile(ast);

        // Debug print module
        module->print(llvm::outs(), nullptr);

        std::cout << "\n";

        // 3. Save the IR to a file
        saveModuleToFile("./output.ll");
    }

private:
    /*
        Compile ast(TODO) to LLVM IR
    */
    void compile(const Expr &ast)
    {
        // 1. Create entry main func
        fn = createFunction("main", llvm::FunctionType::get(builder->getInt32Ty(), false));

        // 2. Compile main
        generate(ast);

        // // 3. Cast ret val to i32 from main
        // auto i32result = builder->CreateIntCast(result, builder->getInt32Ty(), true);

        builder->CreateRet(builder->getInt32(0));
    }

    /*
         Main compile loop, basically interpreter at compile time
    */
    llvm::Value *generate(const Expr &expr)
    {
        switch (expr.type)
        {
        case ExprType::NUMBER:
            return builder->getInt32(expr.number);

        case ExprType::STRING:
        {
            // TODO: Implement all kink of escape sequences at parse level or here
            auto re = std::regex("\\\\n");
            auto str = std::regex_replace(expr.string, re, "\n");

            return builder->CreateGlobalStringPtr(str);
        }

        case ExprType::SYMBOL:
            // TODO: Implement symbol
            return builder->getInt32(0);

        case ExprType::LIST:
            auto tag = expr.list[0];

            // Special cases
            if (tag.type == ExprType::SYMBOL)
            {
                auto op = tag.string;

                if (op == "printf")
                {
                    auto printFn = module->getFunction("printf");

                    std::vector<llvm::Value *> args{};

                    for (auto i = 1; i < expr.list.size(); i++)
                    {
                        args.push_back(generate(expr.list[i]));
                    }

                    return builder->CreateCall(printFn, args);
                }
            }
        }

        // Default return for unreachable code
        return builder->getInt32(0);
    }

    /*
        Define external funcs (from libc++ example)
    */
    void setupExernalFunctions()
    {
        auto bytePtrTy = builder->getInt8Ty()->getPointerTo();
        module->getOrInsertFunction("printf", llvm::FunctionType::get(
                                                  /* Ret type */ builder->getInt32Ty(),
                                                  /* Args */ bytePtrTy,
                                                  /* Does accept Var args */ true));
    }

    /*
        Create a new function in the module
    */
    llvm::Function *createFunction(const std::string &fnName, llvm::FunctionType *fnType)
    {
        // Func prototype might be already present, so
        auto fn = module->getFunction(fnName);
        if (fn == nullptr)
        {
            fn = createFunctionPrototype(fnName, fnType);
        }
        createFunctionBlock(fn);
        return fn;
    }

    /*
        Create a new function prototype in the module (declare, not definition)
    */
    llvm::Function *createFunctionPrototype(const std::string &fnName, llvm::FunctionType *fnType)
    {
        auto fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, fnName, *module);
        verifyFunction(*fn);
        return fn;
    }

    /*
        Create a new function block in the module
    */
    void createFunctionBlock(llvm::Function *fn)
    {
        auto entry = createBB("entry", fn);
        builder->SetInsertPoint(entry);
    }

    /*
         Create a new basic block in the module. Passing 'fn' is optional and would automtically append the block to the parent func. If not passed, it is usable be appending manually via fn->getBasicBlockList().push_back(bb)
    */
    llvm::BasicBlock *createBB(std::string name, llvm::Function *fn = nullptr)
    {
        return llvm::BasicBlock::Create(*ctx, name, fn);
    }

    /*
        Save IR to a file
    */
    void saveModuleToFile(const std::string &filename)
    {
        std::error_code errorCode;
        llvm::raw_fd_ostream dest(filename, errorCode);
        module->print(dest, nullptr);
    }
    /*
        Initialize the Cylli Module
    */
    void moduleInit()
    {
        // Open new ctx & module
        ctx = std::make_unique<llvm::LLVMContext>();
        module = std::make_unique<llvm::Module>("CylliLLVM", *ctx);

        // Create a new IRBuilder based on the ctx
        builder = std::make_unique<llvm::IRBuilder<>>(*ctx);
    }
    /*
        Cylli Parser
    */
    std::unique_ptr<CylliParser> parser;
    /*
        Current compiling function
    */
    llvm::Function *fn;
    /*
        Global LLVM Context
        Owns and manage core's "global" data from LLVM infrastrucure,
        including the type and constant uniquing tables.
     */
    std::unique_ptr<llvm::LLVMContext> ctx;

    /*
        Cylli LLVM Module
        It is the top level main container of all other LLVM IR. Each contain list of globals vars, list of funcs, list of libs for other modules.
        It depends on symbol table and various data about target characteristics for the metadata
        Module maintain GlobalList object used for holding all constant refs to global vars in the module. Means if a global vars is destroyed it will have no entries oon the GlobalList
    */
    std::unique_ptr<llvm::Module> module;

    /*
         IR Builder
         It is used to construct an LLVM IR instruction. It is used to create instructions that are added to basic blocks at end of BasicBlock or at specific location in a block.
    */
    std::unique_ptr<llvm::IRBuilder<>> builder;
};

#endif